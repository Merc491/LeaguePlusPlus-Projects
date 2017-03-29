#include "PluginSDK.h"
#include "Color.h"

PluginSetup("MoeeesAhri");

IMenu* MainMenu;
IMenu* ComboMenu;
IMenu* HarassMenu;
IMenu* LaneClearMenu;
IMenu* JungleClearMenu;
IMenu* RSettings;
IMenu* Drawings;
IMenu* MiscMenu;

IMenuOption* ComboQ;
IMenuOption* ComboW;
IMenuOption* ComboE;
IMenuOption* ComboR;
IMenuOption* RKayle;

IMenuOption* HarassW;
IMenuOption* HarassE;
IMenuOption* HarassQ;
IMenuOption* HarassManaManager;

IMenuOption* LaneClearQ;
IMenuOption* LaneClearManaManager;
IMenuOption* LaneClearMin;

IMenuOption* JungleClearQ;
IMenuOption* JungleClearE;
IMenuOption* JungleClearManaManager;

IMenuOption* autoQ;
IMenuOption* interruptE;
IMenuOption* gapcloseE;

IMenuOption* DrawReady;
IMenuOption* DrawQ;
IMenuOption* DrawW;
IMenuOption* DrawE;
IMenuOption* DrawR;

ISpell2* Q;
ISpell2* W;
ISpell2* E;
ISpell2* R;

IUnit* myHero;


int xOffset = 10;
int yOffset = 15;
int Width = 103;
int Height = 8;
Vec4 Color = Vec4(105, 198, 5, 255);
Vec4 FillColor = Vec4(198, 176, 5, 255);

void Menu()
{
	MainMenu = GPluginSDK->AddMenu("MoeeesAhri");

	ComboMenu = MainMenu->AddMenu("Combo");
	HarassMenu = MainMenu->AddMenu("Harass");
	LaneClearMenu = MainMenu->AddMenu("LaneClear");
	JungleClearMenu = MainMenu->AddMenu("JungleClear");
	Drawings = MainMenu->AddMenu("Drawings");
	MiscMenu = MainMenu->AddMenu("Miscs");

	ComboQ = ComboMenu->CheckBox("Use Q", true);
	ComboW = ComboMenu->CheckBox("Use W", true);
	ComboE = ComboMenu->CheckBox("Use E", true);


	HarassQ = HarassMenu->CheckBox("Use Q", true);
	HarassW = HarassMenu->CheckBox("Use W", true);
	HarassE = HarassMenu->CheckBox("Use E", true);

	LaneClearQ = LaneClearMenu->CheckBox("Use Q", true);
	LaneClearManaManager = LaneClearMenu->AddFloat("ManaManager for Q", 0, 100, 65);
	LaneClearMin = LaneClearMenu->AddInteger("Minimum Minions to use Q", 0, 10, 4);



	autoQ = MiscMenu->CheckBox("Use Q Automatically", true);
	gapcloseE = MiscMenu->CheckBox("Use E on Gap Closers", true);
	interruptE = MiscMenu->CheckBox("Use E for to Interrupt Spells", true);
	

	DrawReady = Drawings->CheckBox("Draw Ready Spells", true);
	DrawQ = Drawings->CheckBox("Draw Q", true);
	DrawW = Drawings->CheckBox("Draw W", true);
	DrawE = Drawings->CheckBox("Draw E", true);
	DrawR = Drawings->CheckBox("Draw R", true);

	

	// by Kornis

}



void LoadSpells()
{

	Q = GPluginSDK->CreateSpell2(kSlotQ, kLineCast, false, false, kCollidesWithNothing);
	Q->SetSkillshot(0.25f, 90, 1550, 870);
	W = GPluginSDK->CreateSpell2(kSlotW, kTargetCast, false, false, kCollidesWithNothing);
	W->SetOverrideRange(580);
	E = GPluginSDK->CreateSpell2(kSlotE, kTargetCast, false, false, static_cast<eCollisionFlags>(kCollidesWithMinions | kCollidesWithYasuoWall));
	E->SetSkillshot(0.25f, 60, 1550, 950);
	R = GPluginSDK->CreateSpell2(kSlotR, kTargetCast, false, false, kCollidesWithNothing);
	R->SetOverrideRange(600);
}



void CastE(IUnit* target)
{
	AdvPredictionOutput prediction_output;
	E->RunPrediction(target, false, kCollidesWithYasuoWall | kCollidesWithMinions, &prediction_output);
	if (prediction_output.HitChance >= kHitChanceHigh)
	{
		E->CastOnTarget(target, kHitChanceCollision);
	}
}

void AntiGapclose(GapCloserSpell const& args)
{
	auto player = GEntityList->Player();
	if (gapcloseE->Enabled() && E->IsReady() && player->IsValidTarget(args.Sender, E->Range()) && args.Sender != nullptr && args.Sender != GEntityList->Player() && args.Sender->IsEnemy(GEntityList->Player()))
	{
		CastE(args.Sender);
	}
}


void AntiInterrupt(InterruptibleSpell const& args)
{
	auto player = GEntityList->Player();
	if (interruptE->Enabled() && E->IsReady() && player->IsValidTarget(args.Target, E->Range()) && args.Target != nullptr && args.Target != GEntityList->Player() && args.Target->IsEnemy(GEntityList->Player()))
	{
		CastE(args.Target);
	}
}


void dmgdraw()
{

	for (auto hero : GEntityList->GetAllHeros(false, true))
	{
		Vec2 barPos = Vec2();
		if (hero->GetHPBarPosition(barPos) && !hero->IsDead())
		{
			float QDamage = 0;
			float WDamage = 0;
			float EDamage = 0;
			float RDamage = 0;
			if (W->IsReady()) {
				WDamage = GDamage->GetSpellDamage(GEntityList->Player(), hero, kSlotW);
			}
			if (Q->IsReady()) {
				QDamage = GDamage->GetSpellDamage(GEntityList->Player(), hero, kSlotQ) + GDamage->GetAutoAttackDamage(GEntityList->Player(), hero, true);
			}
			if (E->IsReady()) {
				EDamage = GDamage->GetSpellDamage(GEntityList->Player(), hero, kSlotE);
			}
			if (R->IsReady()) {
				RDamage = GDamage->GetSpellDamage(GEntityList->Player(), hero, kSlotR);
			}
			float totalDamage = QDamage + WDamage + EDamage + RDamage;
			float percentHealthAfterDamage = max(0, hero->GetHealth() - float(totalDamage)) / hero->GetMaxHealth();
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
	auto target = GTargetSelector->FindTarget(QuickestKill, SpellDamage, E->Range());

	if (target == nullptr || !target->IsHero())
		return;


	if (ComboE->Enabled() && E->IsReady() && player->IsValidTarget(target, E->Range()) && GEntityList->Player()->GetMana() > R->ManaCost() + E->ManaCost())
	{
			CastE(target);
		
		}
	

	if (ComboQ->Enabled() && Q->IsReady() && player->IsValidTarget(target, Q->Range()))
	{
		Q->CastOnTarget(target, kHitChanceVeryHigh);

		}

		if (ComboW->Enabled() && W->IsReady() && player->IsValidTarget(target, W->Range()) && GEntityList->Player()->GetMana() > R->ManaCost() + W->ManaCost())
		{
			W->CastOnPlayer();

		}

	}


void Harass()
{
	auto player = GEntityList->Player();
	auto target = GTargetSelector->FindTarget(QuickestKill, SpellDamage, E->Range());

	if (target == nullptr || !target->IsHero())
		return;


	if (E->IsReady() && player->IsValidTarget(target, E->Range()) && GEntityList->Player()->GetMana() > R->ManaCost() + E->ManaCost())
	{
		CastE(target);

	}

	if (Q->IsReady() && player->IsValidTarget(target, Q->Range()) && GEntityList->Player()->GetMana() > R->ManaCost() + Q->ManaCost())
	{
		Q->CastOnTarget(target, kHitChanceVeryHigh);

	}

	if (HarassW->Enabled() && W->IsReady() && player->IsValidTarget(target, W->Range()) && GEntityList->Player()->GetMana() > R->ManaCost() + W->ManaCost())
	{
		W->CastOnPlayer();

	}


}

void LaneClear()
{ 
	auto player = GEntityList->Player();

	for (auto minion : GEntityList->GetAllMinions(false, true, false))
	{
		if (minion != nullptr && minion->IsValidTarget(GEntityList->Player(), E->Range()) && LaneClearQ->Enabled() && player->ManaPercent() >= LaneClearManaManager->GetFloat())
		{
			Vec3 pos;
			int Qhit;
			GPrediction->FindBestCastPosition(Q->Range(), Q->Radius(), true, true, false, pos, Qhit);
			if (Qhit >= LaneClearMin->GetInteger())
			
			{
				Q->CastOnPosition(pos);
			}
		}
	}
	

}

void JungleClear() {


}



void Automated()
{
	auto player = GEntityList->Player();
	auto target = GTargetSelector->FindTarget(QuickestKill, SpellDamage, E->Range());

	if (autoQ->Enabled() && Q->IsReady() && player->IsValidTarget(target, Q->Range()) && GEntityList->Player()->GetMana() > ( 100 + W->ManaCost() + Q->ManaCost()*2) ) //magic number for r mana cost
	{ 
		IUnit *target = GTargetSelector->FindTarget(QuickestKill, SpellDamage, Q->Range());
		if (target)
		{
			AdvPredictionOutput result;
			Q->RunPrediction(target, false, kCollidesWithNothing, &result);
			if (result.HitChance >= kHitChanceVeryHigh)
			{
				Q->CastOnPosition(result.CastPosition);
			}

		}
	}
}



void Drawing()
{
	auto player = GEntityList->Player();
	if (DrawReady->Enabled())
	{
		if (Q->IsReady() && DrawQ->Enabled())
		{
			GRender->DrawOutlinedCircle(player->GetPosition(), Vec4(225, 225, 0, 225), Q->Range());
		}

		if (W->IsReady() && DrawW->Enabled())
		{
			GRender->DrawOutlinedCircle(player->GetPosition(), Vec4(225, 225, 0, 225), W->Range());
		}

		if (E->IsReady() && DrawE->Enabled())
		{
			GRender->DrawOutlinedCircle(player->GetPosition(), Vec4(225, 225, 0, 225), E->Range());
		}

		if (R->IsReady() && DrawR->Enabled())
		{
			GRender->DrawOutlinedCircle(player->GetPosition(), Vec4(225, 225, 0, 225), R->Range());
		}
	}

	else
	{
		if (DrawQ->Enabled())
		{
			GRender->DrawOutlinedCircle(player->GetPosition(), Vec4(225, 225, 0, 225), Q->Range());
		}

		if (DrawW->Enabled())
		{
			GRender->DrawOutlinedCircle(player->GetPosition(), Vec4(225, 225, 0, 225), W->Range());
		}

		if (DrawE->Enabled())
		{
			GRender->DrawOutlinedCircle(player->GetPosition(), Vec4(225, 225, 0, 225), E->Range());
		}

		if (DrawR->Enabled())
		{
			GRender->DrawOutlinedCircle(player->GetPosition(), Vec4(225, 225, 0, 225), R->Range());
		}
	}
}

PLUGIN_EVENT(void) OnRender()
{
	Drawing();
	dmgdraw();
}


PLUGIN_EVENT(void) OnGameUpdate()
{
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
		LaneClear();
		JungleClear();
	}


	Automated();
	
}


PLUGIN_API void OnLoad(IPluginSDK* PluginSDK)
{
	PluginSDKSetup(PluginSDK);
	Menu();
	LoadSpells();
	Automated();
	GEventManager->AddEventHandler(kEventOnRender, OnRender);
	GEventManager->AddEventHandler(kEventOnGameUpdate, OnGameUpdate);
	GEventManager->AddEventHandler(kEventOnGapCloser, AntiGapclose);
	GEventManager->AddEventHandler(kEventOnInterruptible, AntiInterrupt);


}

PLUGIN_API void OnUnload()
{
	MainMenu->Remove();
	GEventManager->RemoveEventHandler(kEventOnRender, OnRender);
	GEventManager->RemoveEventHandler(kEventOnRender, OnRender);
	GEventManager->RemoveEventHandler(kEventOnGameUpdate, OnGameUpdate);
	GEventManager->RemoveEventHandler(kEventOnInterruptible, AntiInterrupt);


}
