// dllmain.cpp : Defines the entry point for the DLL application.
#include <time.h>
#include "stdafx.h"
#include "Windows.h"
#include "Engine.h"
#include "Hooks.h"
#include <mutex>

#define MAXIMUM_TEXT_SIZE 64U

using namespace std;

CObjectManager* ObjManager;
CFunctions Functions;

bool g_track_enemy_flash = true;
int j_key_flag = 0;
int x_key_flag = 0;
bool is_j_key_ready = true;
bool is_x_key_ready = true;
bool OnStartMessage = true;
bool g_unload = false;

DWORD WINAPI HackThread(HMODULE hModule)
{
	while (Engine::GetGameTime() < 1.0f || !me)
		Sleep(1);

	ObjManager = (CObjectManager*)(*(DWORD*)(baseAddr + oObjManager));

	Functions.PrintChat = (Typedefs::fnPrintChat)(baseAddr + oPrintChat);
	Functions.SendChat = (Typedefs::fnSendChat)(baseAddr + oSendChat);
	Functions.IsTargetable = (Typedefs::fnIsTargetable)(baseAddr + oIsTargetable);
	Functions.IsAlive = (Typedefs::fnIsAlive)(baseAddr + oIsAlive);

	Functions.IsMinion = (Typedefs::fnIsMinion)(baseAddr + oIsMinion);
	Functions.IsTurret = (Typedefs::fnIsTurret)(baseAddr + oIsTurret);
	Functions.IsHero = (Typedefs::fnIsHero)(baseAddr + oIsHero);
	Functions.IsMissile = (Typedefs::fnIsMissile)(baseAddr + oIsMissile);
	Functions.IsNexus = (Typedefs::fnIsNexus)(baseAddr + oIsNexus);
	Functions.IsInhibitor = (Typedefs::fnIsInhibitor)(baseAddr + oIsInhib);
	Functions.IsTroyEnt = (Typedefs::fnIsTroyEnt)(baseAddr + oIsTroy);

	//Functions.CastSpell = (Typedefs::fnCastSpell)((DWORD)GetModuleHandle(NULL) + oCastSpell);
	Functions.GetAttackCastDelay = (Typedefs::fnGetAttackCastDelay)((DWORD)GetModuleHandle(NULL) + oGetAttackCastDelay);
	Functions.GetAttackDelay = (Typedefs::fnGetAttackDelay)((DWORD)GetModuleHandle(NULL) + oGetAttackDelay);

	///////////////////////////////////////////////
	///				\\	Print Chat	//			///	
	//from float to char///////////////////////////
	///					
	///
	////////////////////////////////////////////////////////////////////
																	////
	if (OnStartMessage) {											////
		Engine::PrintChat("================================");		////
		Engine::PrintChat("Sh3n::Flash Timer");		                ////
		Engine::PrintChat("fb.com/MarkLesterDampios101");		    ////
		Engine::PrintChat("================================");		////
		OnStartMessage = true;										////
	}																////
	////////////////////////////////////////////////////////////////////

	while (true)
	{
		auto me_index = me->GetIndex();
		auto gameTime = Engine::GetGameTime();

		if (g_track_enemy_flash) {
			CObject holzer;
			auto obj = holzer.GetFirstObject();

			string msgString = "";
			//collect spell data and check if it is flash
			if (g_track_enemy_flash) {
				if ((GetAsyncKeyState(0x4A) & 0x8000) && (j_key_flag == 0)) {
					j_key_flag = 1;
					is_j_key_ready = true;

					//char* smplmsg = "pressed j";
					//Engine::PrintChat(smplmsg);
				}
				else if (GetAsyncKeyState(0x4A) == 0) {
					j_key_flag = 0;//reset the flag
				}

				if ((GetAsyncKeyState(0x58) & 0x8000) && (x_key_flag == 0)) {
					x_key_flag = 1;
					is_x_key_ready = true;

					//char* smplmsg = "pressed x";
					//Engine::PrintChat(smplmsg);
				}
				else if (GetAsyncKeyState(0x58) == 0) {
					x_key_flag = 0;//reset the flag
				}
			}

			while (obj)
			{
				auto IsHero = obj->IsHero();
				auto Index = obj->GetIndex();
				auto IsEnemyToLocalPlayer = obj->IsEnemyTo(me);

				if (IsHero && (Index != me_index) && IsEnemyToLocalPlayer) {
					auto d_spellSlot = obj->GetSpellSlotByID(4);
					auto d_doneCD = d_spellSlot->IsDoneCD(gameTime);

					auto f_spellSlot = obj->GetSpellSlotByID(5);
					auto f_doneCD = f_spellSlot->IsDoneCD(gameTime);

					auto AttackRange = obj->GetAttackRange();

					//MENU:Track Enemy Flash (J Key) (Self)
					if (g_track_enemy_flash)
					{
						if ((j_key_flag == 1 && is_j_key_ready) || (x_key_flag == 1 && is_x_key_ready)) {

							auto champName = obj->GetChampionName();

							auto d_CDTime = d_spellSlot->GetTime();
							auto d_spellName = d_spellSlot->GetSpellInfo()->GetSpellData()->GetSpellName();

							auto f_CDTime = f_spellSlot->GetTime();
							auto f_spellName = f_spellSlot->GetSpellInfo()->GetSpellData()->GetSpellName();

							float flashCoolDown = 0;
							bool summonerFlashFound = false;

							//Engine::PrintChat(d_spellName);
							//Engine::PrintChat(f_spellName);

							if (strcmp(d_spellName, "SummonerFlash") == 0) {
								if (!d_doneCD) {
									summonerFlashFound = true;
									flashCoolDown = d_CDTime;
								}
							}
							else if (strcmp(f_spellName, "SummonerFlash") == 0) {
								if (!f_doneCD) {
									summonerFlashFound = true;
									flashCoolDown = f_CDTime;
								}
							}

							if (summonerFlashFound) {
								int hour;
								int seconds;
								int minutes;

								//calling Convert function
								Engine::SecondsToClock((int)flashCoolDown, hour, minutes, seconds);

								char str_seconds[MAXIMUM_TEXT_SIZE];
								snprintf(str_seconds, MAXIMUM_TEXT_SIZE, "%d", seconds);

								char str_minutes[MAXIMUM_TEXT_SIZE];
								snprintf(str_minutes, MAXIMUM_TEXT_SIZE, "%d", minutes);

								string champNameString(champName);
								string str_minutes_String(str_minutes);
								string str_seconds_String(str_seconds);
								msgString += champNameString + " " + (minutes < 10 ? "0" : "") + str_minutes_String + "" + (seconds < 10 ? "0" : "") + str_seconds_String + " "; //buffer the string to print out later after iterating all object
							}
						}
					}
				}
				obj = holzer.GetNextObject(obj);
			}

			if (g_track_enemy_flash) {
				if (msgString != "") {
					if (j_key_flag == 1 && is_j_key_ready) {
						Engine::SendChat(msgString.c_str());
					}
					if (x_key_flag == 1 && is_x_key_ready) {
						Engine::PrintChat(msgString.c_str());
					}
				}
			}

			is_j_key_ready = false;
			is_x_key_ready = false;
		}
	}
	while (!g_unload)
		Sleep(1000);

	FreeLibraryAndExitThread(hModule, 0);
	return 0;
}


BOOL APIENTRY DllMain(HMODULE hModule,
	DWORD  ul_reason_for_call,
	LPVOID lpReserved
)
{
	if (hModule != nullptr)
		DisableThreadLibraryCalls(hModule);

	if (ul_reason_for_call == DLL_PROCESS_ATTACH) {
		CreateThread(0, 0, (LPTHREAD_START_ROUTINE)HackThread, 0, 0, 0);
		return TRUE;
	}

	else if (ul_reason_for_call == DLL_PROCESS_DETACH) {
		g_unload = true;
		return TRUE;
	}

	return FALSE;
}