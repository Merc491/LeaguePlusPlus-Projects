#include "PluginSDK.h"


PluginSetup("Moeee's Ahri");

IMenu* MainMenu;
IMenu* ComboMenu;
IMenu* HarassMenu;
IMenu* LaneClearMenu;
IMenu* JungleClearMenu;
IMenu* ksSettings;
IMenu* Drawings;
IMenu* MiscMenu;

IMenuOption* ComboQ;
IMenuOption* ComboW;
IMenuOption* ComboE;

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

IMenuOption* Fleemode;
IMenuOption* autoQ;
IMenuOption* interruptE;
IMenuOption* gapcloseE;
IMenuOption* ComboAA;
IMenuOption* ComboAAkey;
IMenuOption* ComboAALevel;

IMenuOption* killstealQ;
IMenuOption* killstealW;
IMenuOption* killstealE;

IMenuOption* DrawReady;
IMenuOption* DrawQ;
IMenuOption* DrawW;
IMenuOption* DrawE;
IMenuOption* DrawR;
IMenuOption* DrawDmg;
IMenuOption* FlashCondemn;

ISpell2* Q;
ISpell2* W;
ISpell2* E;
ISpell2* R;
ISpell2* Flash;
ISpell2* EFlash;

IUnit* myHero;


int xOffset = 10;
int yOffset = 15;
int Width = 103;
int Height = 8;
Vec4 Color = Vec4(105, 198, 5, 255);
Vec4 FillColor = Vec4(198, 176, 5, 255);

void Menu()
{
	MainMenu = GPluginSDK->AddMenu("Moeee's Ahri");

	ComboMenu = MainMenu->AddMenu("Combo");
	HarassMenu = MainMenu->AddMenu("Harass");
	LaneClearMenu = MainMenu->AddMenu("LaneClear");
	ksSettings = MainMenu->AddMenu("Kill Steal");
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

	killstealQ = ksSettings->CheckBox("Use Q", true);
	killstealW = ksSettings->CheckBox("Use W", true);
	killstealE = ksSettings->CheckBox("Use E", true);

	Fleemode = MiscMenu->AddKey("Flee Mode Key", 75);
	autoQ = MiscMenu->CheckBox("Use Q Automatically", true);
	gapcloseE = MiscMenu->CheckBox("Use E on Gap Closers", true);
	interruptE = MiscMenu->CheckBox("Use E to Interrupt Spells", true);
	ComboAALevel = MiscMenu->AddInteger("At what level disable AA", 1, 18, 6);
	ComboAA = MiscMenu->CheckBox("Disable AA", false);
	ComboAAkey = MiscMenu->AddKey("Disable key", 32);
	FlashCondemn = MiscMenu->AddKey("Flash Charm key", 84);


	DrawDmg = Drawings->CheckBox("Draw Damage Calaclations", true);
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
	Q->SetSkillshot(0.25f, 90, 1550, 840);
	W = GPluginSDK->CreateSpell2(kSlotW, kTargetCast, false, false, kCollidesWithNothing);
	W->SetOverrideRange(580);
	E = GPluginSDK->CreateSpell2(kSlotE, kLineCast, false, false, static_cast<eCollisionFlags>(kCollidesWithMinions | kCollidesWithYasuoWall));
	E->SetSkillshot(0.25f, 60, 1550, 930);
	EFlash = GPluginSDK->CreateSpell2(kSlotE, kLineCast, false, false, static_cast<eCollisionFlags>(kCollidesWithMinions | kCollidesWithYasuoWall));
	EFlash->SetSkillshot(0.25f, 60, 3100, 1350);
	R = GPluginSDK->CreateSpell2(kSlotR, kTargetCast, false, false, kCollidesWithNothing);
	R->SetOverrideRange(600);

	if (strcmp(GEntityList->Player()->GetSpellName(kSummonerSlot1), "SummonerFlash") == 0)
	{
		Flash = GPluginSDK->CreateSpell2(kSummonerSlot1, kCircleCast, false, false, kCollidesWithNothing);
		Flash->SetOverrideRange(425.f);
	}
	if (strcmp(GEntityList->Player()->GetSpellName(kSummonerSlot2), "SummonerFlash") == 0)
	{
		Flash = GPluginSDK->CreateSpell2(kSummonerSlot2, kCircleCast, false, false, kCollidesWithNothing);
		Flash->SetOverrideRange(425.f);
	}
}


static bool IsImmune(IUnit* target)
{
	return target->HasBuff("BlackShield") || target->HasBuff("SivirE") || target->HasBuff("NocturneShroudofDarkness") ||
		target->HasBuff("deathdefiedbuff");

}


void CastE(IUnit* target)
{
	AdvPredictionOutput prediction_output;
	E->RunPrediction(target, false, kCollidesWithYasuoWall | kCollidesWithMinions, &prediction_output);
	if (prediction_output.HitChance >= kHitChanceVeryHigh)
	{
		E->CastOnTarget(target, kHitChanceCollision);
	}
}

void AntiGapclose(GapCloserSpell const& args)
{
	auto player = GEntityList->Player();
	if (gapcloseE->Enabled() && E->IsReady() && player->IsValidTarget(args.Sender, E->Range()) && args.Sender != nullptr && args.Sender != GEntityList->Player() && args.Sender->IsEnemy(GEntityList->Player()))
	{
		E->CastOnTarget(args.Sender, kHitChanceVeryHigh);
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

Vec3 getPosToEflash(Vec3 target)
{
	
	return  GEntityList->Player()->ServerPosition().Extend(GGame->CursorPosition(), Flash->Range());
}

void CastFlash() {
	auto target = GTargetSelector->GetFocusedTarget() != nullptr
		? GTargetSelector->GetFocusedTarget()
		: GTargetSelector->FindTarget(QuickestKill, SpellDamage, EFlash->Range());
	Flash->CastOnPosition(getPosToEflash(target->GetPosition()));
}

void PerformFlashCharm()
{
	GGame->IssueOrder(GEntityList->Player(), kMoveTo, GGame->CursorPosition());
	if (E->IsReady() && Flash->IsReady())
	{
		auto target = GTargetSelector->GetFocusedTarget() != nullptr
			? GTargetSelector->GetFocusedTarget()
			: GTargetSelector->FindTarget(QuickestKill, SpellDamage, EFlash->Range());


		auto flashPosition = GEntityList->Player()->ServerPosition().Extend(GGame->CursorPosition(), Flash->Range());
		AdvPredictionOutput result;
		EFlash->RunPrediction(target, false, kCollidesWithMinions, &result);

		if (target != nullptr && target->IsValidTarget() && !target->IsDead() && !target->IsInvulnerable() && result.HitChance >= kHitChanceVeryHigh)
		{
			EFlash->CastOnTarget(target, kHitChanceVeryHigh);
			GPluginSDK->DelayFunctionCall(200+(GGame->Latency())/2, []() { CastFlash(); });





		}
	}
}


void dmgdraw()
{
	if (DrawDmg->Enabled()) {
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
}

void Combo()
{
	auto player = GEntityList->Player();
	auto target = GTargetSelector->FindTarget(QuickestKill, SpellDamage, E->Range());

	if (target == nullptr || !target->IsHero())
		return;


	if (ComboE->Enabled() && E->IsReady() && player->IsValidTarget(target, E->Range()) && GEntityList->Player()->GetMana() > R->ManaCost() + E->ManaCost() && !IsImmune(target))
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


	if (E->IsReady() && player->IsValidTarget(target, E->Range()) && GEntityList->Player()->GetMana() > R->ManaCost() + E->ManaCost() && !IsImmune(target))
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
		if (minion != nullptr && minion->IsValidTarget(GEntityList->Player(), Q->Range()) && LaneClearQ->Enabled() && player->ManaPercent() >= LaneClearManaManager->GetFloat())
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

void FleeMode()
{
	GGame->IssueOrder(GEntityList->Player(), kMoveTo, GGame->CursorPosition());
	if (Q->IsReady())
		Q->CastOnPosition(GEntityList->Player()->GetPosition().Extend(GGame->CursorPosition(), -400));

}


void killSteal() {

	auto player = GEntityList->Player();
	auto target = GTargetSelector->FindTarget(QuickestKill, SpellDamage, E->Range());


	if (target == nullptr || !target->IsHero())
		return;


	if (killstealE->Enabled() && E->IsReady() && player->IsValidTarget(target, E->Range()) && GDamage->GetSpellDamage(GEntityList->Player(), target, kSlotE) > target->GetHealth() && !IsImmune(target))
	{
		CastE(target);

	}


	if (killstealQ->Enabled() && Q->IsReady() && player->IsValidTarget(target, Q->Range()) && GDamage->GetSpellDamage(GEntityList->Player(), target, kSlotQ) > target->GetHealth())
	{
		Q->CastOnTarget(target, kHitChanceVeryHigh);

	}

	if (killstealW->Enabled() && W->IsReady() && player->IsValidTarget(target, W->Range()) && GDamage->GetSpellDamage(GEntityList->Player(), target, kSlotW) > target->GetHealth())
	{
		W->CastOnPlayer();

	}

}




void Automated()
{
	auto player = GEntityList->Player();
	auto target = GTargetSelector->FindTarget(QuickestKill, SpellDamage, E->Range());

	if (autoQ->Enabled() && Q->IsReady() && player->IsValidTarget(target, Q->Range()) && GEntityList->Player()->GetMana() > (100 + W->ManaCost() + Q->ManaCost() * 2)) //magic number for r mana cost
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

	if (GetAsyncKeyState(FlashCondemn->GetInteger()) && !GGame->IsChatOpen())
	{
		PerformFlashCharm();
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
	}

	if (GetAsyncKeyState(Fleemode->GetInteger()))
	{
		FleeMode();
	}

	if (GetAsyncKeyState(ComboAAkey->GetInteger()))
	{
		auto level = GEntityList->Player()->GetLevel();
		if (ComboAA->Enabled() && level >= ComboAALevel->GetInteger() && GEntityList->Player()->GetMana() > 100)
		{
			GOrbwalking->SetAttacksAllowed(false);
		}
	}
	if (!GetAsyncKeyState(ComboAAkey->GetInteger()) || GEntityList->Player()->GetMana() < 100)
	{
		{
			GOrbwalking->SetAttacksAllowed(true);
		}
	}

	Automated();
	killSteal();

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
	GGame->PrintChat("<font color=\"#ff4dee\"><b>Moeee's Ahri</b></font><b><font color=\"#FFFFFF\"> Loaded!</font></b>");

}

PLUGIN_API void OnUnload()
{
	MainMenu->Remove();
	GEventManager->RemoveEventHandler(kEventOnRender, OnRender);
	GEventManager->RemoveEventHandler(kEventOnRender, OnRender);
	GEventManager->RemoveEventHandler(kEventOnGameUpdate, OnGameUpdate);
	GEventManager->RemoveEventHandler(kEventOnInterruptible, AntiInterrupt);


}
