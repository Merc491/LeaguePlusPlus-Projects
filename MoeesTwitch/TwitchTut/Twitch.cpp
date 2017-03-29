#include "PluginSDK.h"
#include "Color.h"
#include <algorithm>
#include <sstream>


PluginSetup("Moeee's Twitch");

IMenu* MainMenu;
IMenu* ComboMenu;
IMenu* HarassMenu;
IMenu* JungleClearMenu;
IMenu* RSettings;
IMenu* Drawings;
IMenu* MiscMenu;

IMenuOption* ComboQ;
IMenuOption* ComboW;
IMenuOption* ComboE;
IMenuOption* ComboEstacks;
IMenuOption* ComboEks;
IMenuOption* ComboR;

IMenuOption* HarassW;
IMenuOption* HarassE;
IMenuOption* HarassEstacks;
IMenuOption* HarassManaManager;

IMenuOption* LaneClearE;
IMenuOption* LaneClearManaManager;

IMenuOption* JungleClearQ;
IMenuOption* JungleClearE;
IMenuOption* JungleClearManaManager;

IMenuOption* safeQ;
IMenuOption* recallQ;
IMenuOption* useEOnFlee;
IMenuOption* smiteE;
IMenuOption* expireE;

IMenuOption* DrawReady;
IMenuOption* DrawW;
IMenuOption* DrawE;
IMenuOption* DrawR;
IMenuOption* DrawEdmg;

ISpell2* Q;
ISpell2* W;
ISpell2* E;
ISpell2* R;
ISpell2* reCalling;


int xOffset = 10;
int yOffset = 15;
int Width = 103;
int Height = 8;
Vec4 Color = Vec4(105, 198, 5, 255);
Vec4 FillColor = Vec4(198, 176, 5, 255);

void Menu()
{
	MainMenu = GPluginSDK->AddMenu("Moeee's Twitch");

	ComboMenu = MainMenu->AddMenu("Combo");
	HarassMenu = MainMenu->AddMenu("Harass");
	JungleClearMenu = MainMenu->AddMenu("JungleClear");
	Drawings = MainMenu->AddMenu("Drawings");
	MiscMenu = MainMenu->AddMenu("Miscs");

	ComboQ = ComboMenu->CheckBox("Use Q", true);
	ComboW = ComboMenu->CheckBox("Use W", true);
	ComboE = ComboMenu->CheckBox("Use E", true);
	ComboEstacks = ComboMenu->AddInteger("Use E at X stacks for combo ", 0, 6, 6);
	ComboEks = ComboMenu->CheckBox("Only use E to KS ", false);
	ComboR = ComboMenu->CheckBox("Use R ", true);

	HarassW = HarassMenu->CheckBox("Use W in harass", true);
	HarassE = HarassMenu->CheckBox("Use E in harass", true);
	HarassEstacks = HarassMenu->AddInteger("Use E at X stacks for harass", 0, 6, 6);

	safeQ = MiscMenu->CheckBox("Use Q if 3 or more enemies are collapsing", true);
	recallQ = MiscMenu->CheckBox("Stealth recall", true);
	useEOnFlee = MiscMenu->CheckBox("Automatically E enemy before leaving range", true);
	smiteE = MiscMenu->CheckBox("Automatically smite neutral & epic monsters", true);
	expireE = MiscMenu->CheckBox("Use E before it expires (for truly no brain.exe users)", true);



	DrawReady = Drawings->CheckBox("Draw Ready Spells", true);
	DrawW = Drawings->CheckBox("Draw W", true);
	DrawE = Drawings->CheckBox("Draw E", true);
	DrawEdmg = Drawings->CheckBox("Draw E damage", true);
	DrawR = Drawings->CheckBox("Draw R", true);

	RSettings = MainMenu->AddMenu("R Settings");


	// by Kornis

}



void LoadSpells()
{

	Q = GPluginSDK->CreateSpell2(kSlotQ, kTargetCast, false, false, kCollidesWithNothing);
	W = GPluginSDK->CreateSpell2(kSlotW, kCircleCast, false, true, kCollidesWithYasuoWall);
	W->SetSkillshot(0.25f, 100, 1410, 950);
	E = GPluginSDK->CreateSpell2(kSlotE, kTargetCast, false, false, kCollidesWithNothing);
	E->SetOverrideRange(1200);
	R = GPluginSDK->CreateSpell2(kSlotR, kTargetCast, false, false, kCollidesWithNothing);
	R->SetOverrideRange(975);
	reCalling = GPluginSDK->CreateSpell2(kSlotRecall, kTargetCast, false, false, kCollidesWithNothing);
}

int EnemiesInRange(IUnit* Source, float range)
{
	auto Targets = GEntityList->GetAllHeros(false, true);
	auto enemiesInRange = 0;

	for (auto target : Targets)
	{
		if (target != nullptr && !target->IsDead())
		{
			auto flDistance = (target->GetPosition() - Source->GetPosition()).Length();
			if (flDistance < range)
			{
				enemiesInRange++;
			}
		}
	}
	return enemiesInRange;
}

int EnemiesOnYou(IUnit* Source, float range)
{
	auto Targets = GEntityList->GetAllHeros(false, true);
	auto count = 0;

	for (auto target : Targets)
	{
		if (target != nullptr && !target->IsDead())
		{
			auto waypoints = target->GetWaypointList();
			
			if (waypoints.size() > 1)
			{
				auto position = waypoints[0]; {
					auto pDistance = (position - Source->GetPosition()).Length();
					{
						if (pDistance < range)
							count++;
					}
					

				}
			}
		}
	}
	return count;
}

float eDmg(IUnit* Target) //ty based rembrandt
{
	float InitDamage = 0;
	int StackCount = Target->GetBuffCount("twitchdeadlyvenom");

	if (StackCount == 0) return 0;

	float BonusStackDamage = ((0.25 * GEntityList->Player()->BonusDamage()) + (0.2 * GEntityList->Player()->TotalMagicDamage())) * StackCount;

	if (GEntityList->Player()->GetSpellLevel(kSlotE) == 1)
		InitDamage += 20 + (15 * StackCount) + BonusStackDamage;
	else if (GEntityList->Player()->GetSpellLevel(kSlotE) == 2)
		InitDamage += 35 + (20 * StackCount) + BonusStackDamage;
	else if (GEntityList->Player()->GetSpellLevel(kSlotE) == 3)
		InitDamage += 50 + (25 * StackCount) + BonusStackDamage;
	else if (GEntityList->Player()->GetSpellLevel(kSlotE) == 4)
		InitDamage += 65 + (30 * StackCount) + BonusStackDamage;
	else if (GEntityList->Player()->GetSpellLevel(kSlotE) == 5)
		InitDamage += 80 + (35 * StackCount) + BonusStackDamage;

	/*
	BASE PHYSICAL DAMAGE: 20 / 35 / 50 / 65 / 80
	BONUS DAMAGE PER STACK: 15 / 20 / 25 / 30 / 35 (+ 25% bonus AD) (+ 20% AP)
	*/

	return GDamage->CalcPhysicalDamage(GEntityList->Player(), Target, InitDamage);
}

float qDistance()
{
	float movespeed = GEntityList->Player()->MovementSpeed();
	float timeleftQ = GBuffData->GetEndTime(GEntityList->Player()->GetBuffDataByName("globalcamouflage")) - GGame->Time();
	float radius = movespeed * timeleftQ;
	return radius;
}

void dmgdraw()
{
	if (!DrawEdmg->Enabled())
		return;
	for (auto hero : GEntityList->GetAllHeros(false, true))
	{
		Vec2 barPos = Vec2();
		if (hero->GetHPBarPosition(barPos) && !hero->IsDead() && hero->HasBuff("twitchdeadlyvenom"))
		{
			auto EDamage = eDmg(hero);
			float percentHealthAfterDamage = max(0, hero->GetHealth() - float(EDamage)) / hero->GetMaxHealth();
			float yPos = barPos.y + yOffset;
			float xPosDamage = (barPos.x + xOffset) + Width * percentHealthAfterDamage;
			float xPosCurrentHp = barPos.x + xOffset + Width * (hero->GetHealth() / hero->GetMaxHealth());
			if (!hero->IsDead() && hero->IsValidTarget())
			{
				float differenceInHP = xPosCurrentHp - xPosDamage;
				float pos1 = barPos.x + 9 + (107 * percentHealthAfterDamage);

				for (int i = 0; i < differenceInHP; i++)
				{
					GRender->DrawLine(Vec2(pos1 + i, yPos), Vec2(pos1 + i, yPos + Height), FillColor);
				}
				if (!hero->IsVisible())
				{

				}
			}
		}
	}
}


void Combo()
{
	auto player = GEntityList->Player();
	auto target = GTargetSelector->FindTarget(QuickestKill, PhysicalDamage, E->Range());
	if (target == nullptr || !target->IsHero())
		return;

	if (ComboQ->Enabled() && Q->IsReady() && player->IsValidTarget(target, 650) && GEntityList->Player()->GetMana() > R->ManaCost() + Q->ManaCost())
	{
		Q->CastOnPlayer();
	}

	if (ComboW->Enabled() && W->IsReady() && GEntityList->Player()->GetMana() > (R->ManaCost() + W->ManaCost() + E->ManaCost()) && GDamage->GetAutoAttackDamage(GEntityList->Player(), target, true) * 2 < target->GetHealth() && player->IsValidTarget(target, W->Range()))
	{
		W->CastOnTarget(target, kHitChanceMedium);
	}

	if (ComboE->Enabled() && E->IsReady() && target->HasBuff("twitchdeadlyvenom") && player->IsValidTarget(target, E->Range()))
	{
		if (!ComboEks->Enabled() && (target->GetBuffCount("twitchdeadlyvenom") >= ComboEstacks->GetInteger() || eDmg(target) > target->GetHealth())) {
			E->CastOnPlayer();
		}

		if (ComboEks->Enabled() && eDmg(target) > target->GetHealth()) {
			E->CastOnPlayer();
		}

	}

	if (ComboR->Enabled() && R->IsReady() && player->IsValidTarget(target, 975) && EnemiesInRange(player,975) >= 3)
	{
		R->CastOnPlayer();
	}


}



void Harass()
{
	auto player = GEntityList->Player();
	auto target = GTargetSelector->FindTarget(QuickestKill, PhysicalDamage, E->Range());
	if (target == nullptr || !target->IsHero())
		return;
	if (HarassW->Enabled() && W->IsReady() && GEntityList->Player()->GetMana() > (R->ManaCost() + W->ManaCost() + E->ManaCost()) && GDamage->GetAutoAttackDamage(GEntityList->Player(), target, true) * 2 < target->GetHealth() && player->IsValidTarget(target, W->Range()))
	{
		W->CastOnTarget(target, kHitChanceMedium);
	}
	
	if (HarassE->Enabled() && E->IsReady() && player->IsValidTarget(target, E->Range()) && target->HasBuff("twitchdeadlyvenom") )
	{
		if ((target->GetBuffCount("twitchdeadlyvenom") >= HarassEstacks->GetInteger() || eDmg(target) > target->GetHealth())) {
			E->CastOnPlayer();
		}


	}
}

bool OnPreCast(int Slot, IUnit* Target, Vec3* StartPosition, Vec3* EndPosition)
{
	if (Slot == kSlotRecall && Q->IsReady() && recallQ->Enabled() && !GEntityList->Player()->HasBuff("globalcamouflage"))
	{
		Q->CastOnPlayer();
	}

	return true;
}



void Automated()
{
	auto player = GEntityList->Player();
	if (Q->IsReady() && safeQ->Enabled() && EnemiesOnYou(player, 675) >= 3)
	{
		Q->CastOnPlayer();
	}
	for (auto Enemy : GEntityList->GetAllHeros(false, true))
	{
		auto flDistance = (GEntityList->Player()->GetPosition() - Enemy->GetPosition()).Length();
			if (Enemy->IsValidTarget() && !Enemy->IsDead() && !Enemy->IsClone())
			{
				if (Enemy->HasBuff("twitchdeadlyvenom"))
				{
					if (E->IsReady() && useEOnFlee->Enabled() && flDistance > E->Range() - 100)
					{
						E->CastOnPlayer();
					}
					if (E->IsReady() && expireE->Enabled() && GBuffData->GetEndTime(Enemy->GetBuffDataByName("twitchdeadlyvenom")) - GGame->Time() < 1) {
						E->CastOnPlayer();

					}
				}
			}
		}
	for (auto minion : GEntityList->GetAllMinions(false, false, true))
	{
		if (player->IsValidTarget(minion, E->Range()) && smiteE->Enabled() && E->IsReady() && minion->IsJungleCreep() && (eDmg(minion) > minion->GetHealth()) && E->IsReady())

			E->CastOnPlayer();


	}

	}




void LaneClear()
{


}

void JungleClear() {

	auto player = GEntityList->Player();
	for (auto minion : GEntityList->GetAllMinions(false, false, true))
	{
		if (player->IsValidTarget(minion, E->Range()) && smiteE->Enabled() && E->IsReady() && minion->IsJungleCreep() && (eDmg(minion) > minion->GetHealth()) && E->IsReady())

			E->CastOnPlayer();


	}
}




PLUGIN_EVENT(void) OnRender()
{
	
		dmgdraw();
		if (GEntityList->Player()->HasBuff("globalcamouflage"))
		{
			Vec2 pos;
			double timeleftQ = GBuffData->GetEndTime(GEntityList->Player()->GetBuffDataByName("globalcamouflage")) - GGame->Time();
			GRender->DrawOutlinedCircle(GEntityList->Player()->GetPosition(), Vec4(255, 0, 0, 255), qDistance());
			if (GGame->Projection(GEntityList->Player()->GetPosition(), &pos));
				GRender->DrawTextW(Vec2(pos.x + 52, pos.y + 10), Vec4(255, 0, 0, 255), std::to_string(timeleftQ).c_str());
			}
			

		
	

	if (DrawReady->Enabled())
	{
		if (W->IsReady() && DrawW->Enabled()) { GRender->DrawOutlinedCircle(GEntityList->Player()->GetPosition(), Vec4(255, 255, 0, 255), W->Range()); }

		if (E->IsReady() && DrawE->Enabled()) { GRender->DrawOutlinedCircle(GEntityList->Player()->GetPosition(), Vec4(255, 255, 0, 255), E->Range()); }

		if (R->IsReady() && DrawR->Enabled()) { GRender->DrawOutlinedCircle(GEntityList->Player()->GetPosition(), Vec4(255, 255, 0, 255), R->Range()); }

	}
	else
	{
		if (DrawW->Enabled()) { GRender->DrawOutlinedCircle(GEntityList->Player()->GetPosition(), Vec4(255, 255, 0, 255), W->Range()); }

		if (DrawE->Enabled()) { GRender->DrawOutlinedCircle(GEntityList->Player()->GetPosition(), Vec4(255, 255, 0, 255), E->Range()); }

		if (DrawR->Enabled()) { GRender->DrawOutlinedCircle(GEntityList->Player()->GetPosition(), Vec4(255, 255, 0, 255), R->Range()); }
	}
}
	



PLUGIN_EVENT(void) OnGameUpdate()
{
	Automated();

	if (GOrbwalking->GetOrbwalkingMode() == kModeCombo)
	{
		Combo();
	}

	if (GOrbwalking->GetOrbwalkingMode() == kModeMixed)
	{
		Harass();
	}

	if (GOrbwalking->GetOrbwalkingMode() == kModeLaneClear)
	{
		JungleClear();
	}



}


PLUGIN_API void OnLoad(IPluginSDK* PluginSDK)
{
	PluginSDKSetup(PluginSDK);
	Menu();
	LoadSpells();
	GEventManager->AddEventHandler(kEventOnRender, OnRender);
	GEventManager->AddEventHandler(kEventOnGameUpdate, OnGameUpdate);
	GEventManager->AddEventHandler(kEventOnPreCast, OnPreCast);
	GGame->PrintChat("<font color=\"#62ff4d\"><b>Moeee's Twitch V1.4</b></font><b><font color=\"#FFFFFF\"> Loaded!</font></b>");


}

PLUGIN_API void OnUnload()
{
	MainMenu->Remove();
	GEventManager->RemoveEventHandler(kEventOnRender, OnRender);
	GEventManager->RemoveEventHandler(kEventOnGameUpdate, OnGameUpdate);
	GEventManager->RemoveEventHandler(kEventOnPreCast, OnPreCast);

}

