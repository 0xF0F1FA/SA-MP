//----------------------------------------------------------
//
// SA:MP Multiplayer Modification For GTA:SA
// Copyright 2004-2005 SA:MP team
//
// Version: $Id: hooks.cpp,v 1.45 2006/05/08 17:35:55 kyeman Exp $
//
//----------------------------------------------------------

#include "../main.h"
#include "keystuff.h"

extern DWORD dwGraphicsLoop; // Used for the external dll game loop.

#define NUDE void _declspec(naked) 

//-----------------------------------------------------------
// Globals which are used to avoid stack frame alteration
// inside the following hook procedures.

GTA_CONTROLSET *pGcsKeys;

// used generically
PED_TYPE	*_pPlayer;
VEHICLE_TYPE *_pVehicle;
MATRIX4X4	_matVehicle;
VECTOR		_vecVehicleMoveSpeed;
BYTE	byteInternalPlayer=0;
DWORD	dwCurPlayerActor=0;
BYTE	byteCurPlayer=0;
BYTE	byteSavedCameraMode;
WORD	wSavedCameraMode2;
BYTE	*pbyteCameraMode = (BYTE *)0xB6F1A8;
BYTE	*pbyteCurrentPlayer = (BYTE *)0xB7CD74;
WORD    *wCameraMode2 = (WORD*)0xB6F858;

DWORD	dwSavedEcx=0;
int		iRadarColor1=0;
DWORD	dwSavedCheatFn=0;

DWORD	vtbl;
DWORD	call_addr;

//char s[256];

float fFarClip = 1400.0f;

DWORD dwParam1;

DWORD dword_1018F56C;
DWORD dwAnimID1Saved;
DWORD dwAnimID2Saved;

//-----------------------------------------------------------
// x86 codes to perform our unconditional jmp for detour entries. 

BYTE Hook_7_HookJmpCode[] = { 0xFF,0x25,0x34,0x39,0x4D,0x00,0x90,0x90,0x90,0x90 };
BYTE Hook_8_HookJmpCode[] = { 0xFF,0x25,0x09,0x46,0x4D,0x00,0x90 };
BYTE Hook_24_HookJmpCode[] = { 0xFF,0x25,0xBB,0x19,0x69,0x00,0x90 };
BYTE Hook_25_HookJmpCode[] = { 0xFF,0x25,0xBA,0xB8,0x63,0x00,0x90 };
BYTE RadarTranslateColor_HookJmpCode[] = {0xFF,0x25,0x79,0x4A,0x58,0x00,0x90}; // 584A79
BYTE CheatProcessHook_JmpCode[] = {0xFF,0x25,0xAA,0x85,0x43,0x00,0x90}; // 4385AA
BYTE CGameShutdown_HookJmpCode[] = {0xFF,0x25,0xF1,0xC8,0x53,0x00,0x90}; // 53C8F1
BYTE Hook_29_HookJmpCode[] = { 0xFF,0x25,0xBC,0x5A,0x4B,0x00 };
BYTE CProjectileInfo_Update_HookJmpCode[] = { 0xFF, 0x25, 0x1B, 0x8B, 0x73, 0x00 }; //00738B1B
BYTE CWeapon__Satchel__Activate_HookJmpCode[] = { 0xFF, 0x25, 0x5B, 0x88, 0x73, 0x00 }; // 0073885B
BYTE GetText_HookJmpCode[] = { 0xFF, 0x25, 0x43, 0x00, 0x6A, 0x00, 0x90, 0x90, 0x90 }; //, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90}; // 0x6A004325
BYTE Hook_36_HookJmpCode[] = { 0xFF,0x25,0x21,0xC7,0x59,0x00 };
BYTE Hook_40_HookJmpCode[] = { 0xFF,0x25,0x84,0x80,0x53,0x00,0x90 };
BYTE Hook_48_HookJmpCode[] = { 0xFF,0x25,0xA6,0x34,0x55,0x00,0x90,0x90,0x90 };
BYTE KillEvent_HookJmpCode[] = { 0xFF,0x25,0x33,0x34,0x4B,0x00 }; // 4B3433
BYTE Hook_60_HookJmpCode[] = { 0xFF,0x25,0xDB,0x74,0x56,0x00 };

//-----------------------------------------------------------

/*
NUDE GameProcessHook()
{
	if(pGame && !pGame->IsMenuActive()) {
		if(pNetGame && pNetGame->GetTextDrawPool())	pNetGame->GetTextDrawPool()->Draw();
	}
	_asm add esp, 190h
	_asm ret	
}*/

//-----------------------------------------------------------

extern bool bWantHudScaling;

float fScreenWidthScale;
float fScreenHeightScale;
float fScreenWidth;
float fScreenHeight;

void UpdateHudScales()
{
	fScreenWidthScale = *(float*)0x859520;
	fScreenHeightScale = *(float*)0x859524;
	fScreenWidth = *(float*)0xC17044;
	fScreenHeight = *(float*)0xC17048;

	if (bWantHudScaling)
	{
		if (fScreenWidth / fScreenHeight >= 1.6f)
		{
			*(float*)0x859524 = 0.002420f;
			*(float*)0x859520 = 0x00222f / (fScreenWidth / fScreenHeight);
		}
	}
}

NUDE CHud_DrawRadar_Hook()
{
	_asm pushad

	UpdateHudScales();

	_asm popad
	_asm mov edx, 0x58A330
	_asm call edx
	_asm pushad

	*(float*)0x859520 = fScreenWidthScale;
	*(float*)0x859524 = fScreenHeightScale;

	_asm popad
	_asm ret
}

//-----------------------------------------------------------

BOOL bAiming;

NUDE CHud_DrawCrossHairs_Hook()
{
	_asm pushad

	bAiming = FALSE;

	if (pGame && pGame->FindPlayerPed())
	{
		if (pGame->FindPlayerPed()->GetCameraMode() == 53)
		{
			UpdateHudScales();
			bAiming = TRUE;
		}
	}

	_asm popad
	_asm mov edx, 0x58E020
	_asm call edx
	_asm pushad

	if (bAiming)
	{
		*(float*)0x859520 = fScreenWidthScale;
		*(float*)0x859524 = fScreenHeightScale;
		bAiming = FALSE;
	}

	_asm popad
	_asm ret
}

//-----------------------------------------------------------

NUDE CCamera_Process_Hook()
{
	_asm pushad

	if (pGame && pGame->GetCamera())
	{
		pGame->GetCamera()->Update();
	}

	_asm popad

	_asm mov eax, 0x52B730
	_asm jmp eax
}

extern CObjectPool* pDebugObjectPool;

NUDE Hook_4()
{
	if (pNetGame && pNetGame->GetObjectPool())
		pNetGame->GetObjectPool()->Process();

	if (pDebugObjectPool)
		pDebugObjectPool->Process();

	if (pChatWindow)
		pChatWindow->UpdateSurface();

	_asm mov edx, 0x53BEE0
	_asm call edx
}

//-----------------------------------------------------------
extern DWORD dwFogEnabled;
void SetupD3DFog(BOOL bEnable);

NUDE CPed_Render_Hook()
{
	_asm pushad
	SetupD3DFog(FALSE);
	_asm popad

	_asm mov edx, 0x5E7680
	_asm call edx

	_asm pushad

	if(dwFogEnabled) {
		SetupD3DFog(TRUE);
	} else {
		SetupD3DFog(FALSE);
	}

	_asm popad
	_asm ret
}

//-----------------------------------------------------------

/*DWORD dwSayParam1;
DWORD dwSayParam2;
float fSayParam3;
DWORD dwSayParam4;
DWORD dwSayParam5;
DWORD dwSayParam6;

NUDE CPed_Say_Hook()
{
    _asm mov eax, [esp+4]
	_asm mov dwSayParam1, eax
	_asm mov eax, [esp+8]
	_asm mov dwSayParam2, eax
	_asm mov eax, [esp+12]
	_asm mov fSayParam3, eax
	_asm mov eax, [esp+16]
	_asm mov dwSayParam4, eax
	_asm mov eax, [esp+20]
	_asm mov dwSayParam5, eax
	_asm mov eax, [esp+24]
	_asm mov dwSayParam6, eax

	_asm pushad

	if(dwSayParam1 != 45) {
		if(pChatWindow) pChatWindow->AddDebugMessage("CPed::Say(%u,%u,%f,%u,%u,%u)",
			dwSayParam1,dwSayParam2,fSayParam3,dwSayParam4,dwSayParam5,dwSayParam6);
	}

	_asm popad

    _asm mov eax, [esp+4]
    _asm test ax, ax
	_asm mov edx, 0x5EFFE7
	_asm jmp edx
}*/

//-----------------------------------------------------------

NUDE FrameUpdate_Hook()
{
	Sleep(1);
	
	_asm mov edx, 0x561A80
	_asm jmp edx
}

NUDE Hook_7() {}
NUDE Hook_8() {}
NUDE Hook_9() {}
NUDE Hook_10() {}
NUDE Hook_11() {}

//-----------------------------------------------------------

DWORD dwWeaponSkillIndex;
BOOL bWeaponSkillChanged=FALSE;

NUDE CPed_GetWeaponSkillIndex_Hook()
{
	_asm mov dwCurPlayerActor, ecx

	_pPlayer = (PED_TYPE*)dwCurPlayerActor;
	byteInternalPlayer = *pbyteCurrentPlayer; // get the current internal player number
	byteCurPlayer = FindPlayerNumFromPedPtr(dwCurPlayerActor); // get the ordinal of the passed actor

	if( dwCurPlayerActor &&
		(byteCurPlayer != 0) &&
		(byteInternalPlayer == 0)) // not local player and local player's keys set.
	{
		GameStoreLocalPlayerWeaponSkills();
		GameSetRemotePlayerWeaponSkills(byteCurPlayer);
		bWeaponSkillChanged = TRUE;
	}

	_asm mov ecx, dwCurPlayerActor
	_asm movsx eax, byte ptr [ecx+1816]
	_asm imul eax, 28
	_asm mov eax, [eax+ecx+1440]
	_asm push eax
	_asm mov edx, 0x5E3B60
	_asm call edx
	_asm mov dwWeaponSkillIndex, eax

	if (bWeaponSkillChanged)
	{
		GameSetLocalPlayerWeaponSkills();
		bWeaponSkillChanged = FALSE;
	}

	_asm mov eax, dwWeaponSkillIndex
	_asm ret
}

//-----------------------------------------------------------
// We use a special bit (32) on dwProcFlags (+28) to indicate
// whether we should process gravity/collisions on this PlayerPed.

NUDE CPlayerPed_ProcessCollision_Hook()
{
	_asm test ecx, ecx
	_asm jnz ptr_is_ok
	_asm ret
ptr_is_ok:
	_asm mov eax, [ecx+28]
	_asm shr eax, 31
	_asm cmp eax, 1
	_asm jne do_process_cols
	_asm ret // we set top bit so don't process this
do_process_cols:
	_asm mov edx, 0x54DFB0
	_asm jmp edx
}

//-----------------------------------------------------------

NUDE Hook_14() {}
NUDE Hook_15() {}

//-----------------------------------------------------------

BYTE byteSaveControlFlags;

NUDE Hook_16() // TODO
{
	_asm mov _pVehicle, ecx
	_asm mov eax, [ecx]
	_asm mov vtbl, eax
	_asm pushad

	if(vtbl == 0x871120) { // AUTOMOBILE
		call_addr = 0x6B1880;
	}
	else if(vtbl == 0x8721A0) { // BOAT
		call_addr = 0x6F1770;
	}
	else if(vtbl == 0x871360) {	// MOTORBIKE
		call_addr = 0x6B9250;
	}
	else if(vtbl == 0x871948) { // PLANE
		call_addr = 0x6C9260;
	}
	else if(vtbl == 0x871680) { // HELI
		call_addr = 0x6C7050;
	}
	else if(vtbl == 0x871528) { // PUSHBIKE (BMX?)
		call_addr = 0x6BFA30;
	}
	else if(vtbl == 0x8717d8) { // UNKNOWN2
		call_addr = 0x6C8250;
	}
	else if(vtbl == 0x871AE8) { // UNKNOWN1
		call_addr = 0x6CDCC0;
	}
	else if(vtbl == 0x872370) { // TRAIN
		call_addr = 0x6F86A0;
	}
	else if(vtbl == 0x871C28) { // TRAILER
		call_addr = 0x6CED20;
	}

	byteInternalPlayer = *(BYTE*)0xB7CD74;

	if ((_pVehicle->pDriver) && (_pVehicle->pDriver->dwPedType == 0) &&
		(_pVehicle->pDriver != GamePool_FindPlayerPed()) &&
		(byteInternalPlayer == 0)) // not player's car
	{
		byteCurPlayer = FindPlayerNumFromPedPtr((DWORD)_pVehicle->pDriver);

		GameStoreLocalPlayerKeys(); // save local player keys
		GameSetRemotePlayerKeys(byteCurPlayer); // set remote player keys.

		// save the internal cammode, apply the context.
		byteSavedCameraMode = *pbyteCameraMode;
		*pbyteCameraMode = GameGetPlayerCameraMode(byteCurPlayer);

		// save the second internal cammode, apply the context
		wSavedCameraMode2 = *wCameraMode2;
		*wCameraMode2 = GameGetPlayerCameraMode(byteCurPlayer);
		if(*wCameraMode2 == 4) *wCameraMode2 = 0;

		// aim switching
		GameStoreLocalPlayerAim();
		GameSetRemotePlayerAim(byteCurPlayer);

		if(_pVehicle->byteFlags & 0x10) {
			_pVehicle->entity.nControlFlags &= 0xDF;
		} else {
			if (vtbl == 0x871680)
				_pVehicle->entity.nControlFlags |= 0x20;

			pGcsKeys = GameGetInternalKeys();
			pGcsKeys->wKeys1[14] = 0;
			pGcsKeys->wKeys2[14] = 0;
			pGcsKeys->wKeys1[16] = 0;
			pGcsKeys->wKeys2[16] = 0;
		}

		// VEHICLE ENGINE AUDIO/RADIO
		*pbyteCurrentPlayer = 0;
		_pVehicle->pDriver->dwPedType = 4; // So CPed::IsPlayer returns FALSE
		byteSaveControlFlags = _pVehicle->entity.nControlFlags;
		_pVehicle->entity.nControlFlags = 0x1A;

		_asm mov edx, _pVehicle
		_asm lea ecx, [edx+312]
		_asm mov edx, 0x502280 // ProcessVehicleAudio
		_asm call edx

		_pVehicle->entity.nControlFlags = byteSaveControlFlags;
		_pVehicle->pDriver->dwPedType = 0;

		*pbyteCurrentPlayer = byteCurPlayer;

		if (_pVehicle->entity.mat) {
			memcpy(&_matVehicle,_pVehicle->entity.mat,sizeof(MATRIX4X4));
			_vecVehicleMoveSpeed.X = _pVehicle->entity.vecMoveSpeed.X;
			_vecVehicleMoveSpeed.Y = _pVehicle->entity.vecMoveSpeed.Y;
			_vecVehicleMoveSpeed.Z = _pVehicle->entity.vecMoveSpeed.Z;
		}

		// CVehicle*::ProcessControl
		_asm mov ecx, _pVehicle
		_asm mov eax, call_addr
		_asm call eax


	}

	_asm popad

}

//-----------------------------------------------------------

VEHICLE_TYPE *_pHornVehicle;
//int _iHasSetHornHookFix = 0;
BYTE _byteSavedControlFlags = 0;
DWORD _dwVehicleParams = 0;
DWORD _dwAudioClass = 0;

NUDE VehicleHorn_Hook()
{
	_asm mov _dwAudioClass, ecx

	_asm mov edx, [esp+4]
	_asm mov _dwVehicleParams, edx

	_asm mov eax, [edx+16]	
	_asm mov _pHornVehicle, eax

	_byteSavedControlFlags = _pHornVehicle->entity.nControlFlags;

	if( _pHornVehicle->pDriver && IN_VEHICLE(_pHornVehicle->pDriver) )
		_pHornVehicle->entity.nControlFlags = 0x02;
	else
		_pHornVehicle->entity.nControlFlags = 0x22;

	_asm push _dwVehicleParams
	_asm mov ecx, _dwAudioClass
	_asm mov edx, 0x5002C0
	_asm call edx

	_pHornVehicle->entity.nControlFlags = _byteSavedControlFlags;

	_asm retn 4
}

//-----------------------------------------------------------

NUDE ZoneOverlay_Hook()
{
	_asm pushad;
	if (pNetGame && pNetGame->GetGangZonePool()) pNetGame->GetGangZonePool()->Draw();
	_asm popad;
	_asm ret;
}

//-----------------------------------------------------------

NUDE PlayerWalk_Hook()
{
	_asm pushad;
	if (pNetGame && pNetGame->GetWalkStyle())
	{
		_asm popad;
		_asm mov[esi+0x4D4], eax;
	}
	else
	{
		_asm popad;
	}
	_asm ret;
}

//-----------------------------------------------------------

NUDE PickUpPickup_Hook()
{
	_asm mov dwParam1, esi
	_asm pushad;

	if (pNetGame && pNetGame->GetPickupPool()) {
		CPickupPool* pPickups = pNetGame->GetPickupPool( );
		pPickups->PickedUp( ((dwParam1-0x9788C0) / 0x20) );
	}

	_asm popad;
	_asm mov al, [esi+0x1C]
	_asm cmp al, 6
	_asm push 0x4579CB
	_asm ret
}

//-----------------------------------------------------------

NUDE CWeapon_FireCamera_Hook()
{
	_asm mov ebx, [esp+8]
	_asm mov dwCurPlayerActor, ebx
	_asm pushad

	if (dwCurPlayerActor == (DWORD)GamePool_FindPlayerPed()) {
		*(BYTE*)0xC8A7C0 = 1;
		*(BYTE*)0xC8A7C1 = 1;
	}

	_asm popad
	_asm mov ebx, 0x73C260
	_asm jmp ebx
}

//-----------------------------------------------------------

NUDE CCameraCamShake_Sniper_Hook()
{
	//_asm mov dwCurPlayerActor, esi
	//if (dwCurPlayerActor == (DWORD)GamePool_FindPlayerPed()) {
	//	_asm mov eax, 0x50A970
	//	_asm jmp eax
	//}
	_asm ret
}

//-----------------------------------------------------------

NUDE CTrain_ProcessControl_Derailment()
{
	_asm {
		mov eax, [esi+1444];	// get the speed
		and eax, 0x80000000;	// get the upper bit
		or eax, 0x3F800000;		// or it with 1.0f (it'll be -ve if bit is set)
		mov [esi+1444], eax;	// set the new speed
		ret;
	}
}

//-----------------------------------------------------------

NUDE Hook_24() {}
NUDE Hook_25() {}

//-----------------------------------------------------------

NUDE CheatProcessorHook()
{
	_asm mov dwSavedCheatFn, eax
	_asm pushad

	//sprintf(s,"CheatFn: 0x%X\n",dwSavedCheatFn);
	//OutputDebugString(s);

	_asm mov edx, 0x96918C
	_asm mov byte ptr [edx], 1
	_asm popad
	//_asm mov edx, 0x43857D ;(process cheat)
	_asm mov edx, 0x438581 ;(don't process cheat)
	_asm cmp eax, 0
	_asm jmp edx
}

//-----------------------------------------------------------

NUDE RadarTranslateColor()
{
	_asm mov eax, [esp+4]
	_asm mov iRadarColor1, eax
	TranslateColorCodeToRGBA(iRadarColor1); // return will still be in eax.
	_asm ret
}

//-----------------------------------------------------------
// We use this to trap and exit the game

DWORD dwShutDownTick;
void QuitGame();

NUDE CGameShutdownHook()
{
	dwShutDownTick = GetTickCount() + 2000;
	QuitGame();

	while (GetTickCount() < dwShutDownTick) {
		Sleep(100);
	}

	ExitProcess(0);
}

//-----------------------------------------------------------

NUDE Hook_29() {}

//-----------------------------------------------------------

NUDE AnimCrashFixHook()
{
	__asm
	{
		push edi;
		mov edi, [esp+8];	// arg0
		test edi, edi;
		jz exitFn
		mov eax, 0x4D41C5;	// continuation for function
		jmp eax;
exitFn:
		pop edi;
		ret;
	}
}

//-----------------------------------------------------------

extern PED_TYPE CrimeReportPed;

NUDE CrimeReport_Hook()
{
	_asm mov eax, offset CrimeReportPed
	_asm ret
}

//-----------------------------------------------------------

DWORD dwProjectile = NULL;
DWORD dwProjectile_Pool_Entry = NULL;

NUDE CProjectile_Update_Hook()
{
	_asm mov dwProjectile, ESI
	_asm mov dwProjectile_Pool_Entry, EBX
	_asm pushad

	if ( *(DWORD*)(dwProjectile_Pool_Entry + 4) == NULL ) // If a crash would have happened
	{
		__asm
		{
			PUSH dwProjectile
			MOV EAX, 0x739A40
			CALL EAX			// CProjectileInfo::Remove
			ADD ESP,4

			_asm popad

			mov dl, 0xFF
		}
	} else {
		_asm popad

		_asm mov ecx, [ebx+4]
		_asm mov dl, [ecx+36h]
	}

	_asm PUSH 0x00738F40
	_asm RETN
}

//-----------------------------------------------------------

NUDE CWeapon__Satchel__Activate_Hook()
{
	__asm
	{
		CMP DWORD PTR DS:[ESI-0x20], 0x27
		MOV EDI, DWORD PTR SS:[EBP]
		JNZ skip
		MOV EAX, [ESP]
		CMP DWORD PTR DS:[ESI-0x1C], EAX
		JNZ skip
		MOV EAX, 0x738880
		JMP EAX

skip:
		MOV EAX, 0x7388DB
		JMP EAX
	}
}

//-----------------------------------------------------------

PCHAR szGotText;

NUDE GetText_Hook()
{
	_asm sub esp, 0x20
	_asm push esi
	_asm push edi
	_asm mov edi, [esp + 0x2C]
	_asm pushad
	_asm mov dwSavedEcx, edi

	szGotText = NULL;
	// Faster than doing strncmp
	if (pNetGame && *(PCHAR)(dwSavedEcx) == 'S' && *(PCHAR)(dwSavedEcx + 1) == 'A' && *(PCHAR)(dwSavedEcx + 2) == 'M' && *(PCHAR)(dwSavedEcx + 3) == 'P')
	{
		if (pNetGame->GetMenuPool()) szGotText = pNetGame->GetMenuPool()->GetTextPointer((PCHAR)(dwSavedEcx + 4));
	}

	_asm popad
	if (szGotText) {
		_asm mov eax, szGotText
		_asm pop edi
		_asm pop esi
		_asm add esp, 0x20
		_asm retn 4
	}

	_asm mov eax, 0x6A0059
	_asm jmp eax
}

//-----------------------------------------------------------

NUDE Hook_35() {}

// RwRasterDestroy
NUDE Hook_36() {}

//-----------------------------------------------------------

DWORD dwLastRenderedVehicle=NULL;

NUDE UpdateVehicleTexture_Hook()
{
	_asm mov dwLastRenderedVehicle, esi
	_asm ret
}

//-----------------------------------------------------------

NUDE Hook_38() {}

//-----------------------------------------------------------

VECTOR vecPlayerVelocity;
CPlayerPed* _pPlayerPed;

NUDE GetPlayerVelocity_Hook()
{
	if( pGame &&
		(_pPlayerPed=pGame->FindPlayerPed()) != NULL &&
		_pPlayerPed->IsInVehicle() &&
		_pPlayerPed->IsAPassenger() )
	{
		_asm lea eax, vecPlayerVelocity
		_asm ret
	}

	_asm mov edx, 0x56E090
	_asm jmp edx
}

//-----------------------------------------------------------

DWORD dwObjectInstance;
PCHAR szObjectModelName;

NUDE Hook_40()
{
	_asm mov eax, [esp+4]
	_asm mov dwObjectInstance, eax
	_asm mov eax, [esp+8]
	_asm mov szObjectModelName, eax
	_asm pushad

	{
		// TODO
	}

	_asm popad
	_asm push 0xFFFFFFFF
	_asm push 0x83C931
	_asm mov eax, 0x538097
	_asm jmp eax
}

//-----------------------------------------------------------

float fRange1Saved;
float fRange2Saved;

NUDE UpdateEscalator_Hook()
{
	_asm pushad
	
	//UnFuck(0x858BA4,4);
	//UnFuck(0x858F84,4);
	fRange1Saved = *(float*)0x858BA4;
	fRange2Saved = *(float*)0x858F84;
	*(float*)0x858BA4 = 40.0f;
	*(float*)0x858F84 = 50.0f;

	_asm popad
	_asm mov eax, 0x717D30
	_asm call eax
	_asm pushad

	*(float*)0x858BA4 = fRange1Saved;
	*(float*)0x858F84 = fRange2Saved;

	_asm popad
	_asm ret
}

//-----------------------------------------------------------

DWORD _dwRetAddr;
BYTE bytePatchDestroyBuilding[] = { 0xFF,0x52,0x20 };

NUDE CreateRwObject_Hook()
{
	_asm mov eax, [esp]
	_asm mov _dwRetAddr, eax
	_asm pushad

	//UnFuck(0x4096AA, 3);
	memset((PVOID)0x4096AA, 0x90, 3);

	_asm popad
	_asm mov eax, 0x533D30
	_asm call eax ; CEntity__CreateRwObject
	_asm pushad

	memcpy((PVOID)0x4096AA, bytePatchDestroyBuilding, sizeof(bytePatchDestroyBuilding));

	_asm popad
	_asm ret
}

//-----------------------------------------------------------

BYTE bytePatchEscalatorOff[] = {0xE8,0x3C,0xFF,0xFF,0xFF};
ENTITY_TYPE* _pObjectEntity;

NUDE DeleteRwObject_Hook()
{
	_asm mov eax, [esp]
	_asm mov _dwRetAddr, eax
	_asm mov _pObjectEntity, ecx
	_asm pushad

	// is it an escalator object?
	if( _pObjectEntity->nModelIndex == 3586 ||
		_pObjectEntity->nModelIndex == 3743 ||
		_pObjectEntity->nModelIndex == 8980 ||
		_pObjectEntity->nModelIndex == 8979 )
	{
		//UnFuck(0x71791F,5);
		memset((PVOID)0x71791F,0x90,5);

		_asm popad
		_asm mov eax, 0x534030
		_asm call eax
		_asm pushad

		memcpy((PVOID)0x71791F,bytePatchEscalatorOff,sizeof(bytePatchEscalatorOff));

		_asm popad
		_asm ret
	}

	_asm popad
	_asm mov eax, 0x534030
	_asm jmp eax
}

//-----------------------------------------------------------

NUDE Hook_44() {}
NUDE Hook_45() {}
NUDE Hook_46() {}
NUDE Hook_47() {}
NUDE Hook_48() {}

//-----------------------------------------------------------

NUDE RoadsignCrashFixHook()
{
	_asm mov eax, [esp+4]
	_asm test eax, eax
	_asm jz exitFn
	_asm mov eax, 0x6FF350
	_asm jmp eax
exitFn:
	_asm ret
}

//-----------------------------------------------------------

DWORD dwThisEvent;
PED_TYPE* pEventDamagePed;

NUDE KillEvent_Hook()
{
	_asm mov dwThisEvent, ecx
	_asm mov eax, [esp+4]
	_asm mov pEventDamagePed, eax
	_asm pushad

	if (pNetGame && pEventDamagePed == GamePool_FindPlayerPed() && *pbyteCurrentPlayer)
	{
		_asm popad
		_asm xor eax, eax
		_asm mov al, 1
		_asm retn 4
	}

	_asm popad
	_asm sub esp, 12
	_asm push esi
	_asm mov esi, ecx
	_asm mov eax, 0x4B35A6
	_asm jmp eax
}

//-----------------------------------------------------------

NUDE Hook_51() {}

//-----------------------------------------------------------

NUDE InVehicleCameraCheck_Hook()
{
	_asm pushad

	if (pNetGame && pNetGame->m_bRemoteVehicleCollisions)
	{
		_asm popad
		_asm xor eax, eax
		_asm ret
	}

	_asm popad
	_asm mov eax, 0x41A990
	_asm jmp eax
}

//-----------------------------------------------------------

NUDE Hook_53()
{
	_asm mov eax, [esp+4]
	_asm test eax, eax
	_asm jz exitFn
	_asm mov ecx, [eax+252]
	_asm cmp ecx, 0
	_asm jnz exitFn
	_asm mov edx, 0x563F40
	_asm jmp edx
exitFn:
	_asm mov eax, 1
	_asm ret
}

//-----------------------------------------------------------

NUDE Hook_54() {}
NUDE Hook_55() {}
NUDE Hook_56() {}
NUDE Hook_57() {}
NUDE Hook_58() {}
NUDE Hook_59() {}
NUDE Hook_60() {}

//-----------------------------------------------------------

void InstallMethodHook(	DWORD dwInstallAddress,
						DWORD dwHookFunction )
{
	//DWORD oldProt, oldProt2;
	//VirtualProtect((LPVOID)dwInstallAddress,4,PAGE_EXECUTE_READWRITE,&oldProt);
	*(PDWORD)dwInstallAddress = (DWORD)dwHookFunction;
	//VirtualProtect((LPVOID)dwInstallAddress,4,oldProt,&oldProt2);
}

//-----------------------------------------------------------

void InstallHook( DWORD dwInstallAddress,
				  DWORD dwHookFunction,
				  DWORD dwHookStorage,
				  BYTE * pbyteJmpCode,
				  int iJmpCodeSize )
{
	//DWORD oldProt, oldProt2;

	// Install the pointer to procaddr.
	//VirtualProtect((PVOID)dwHookStorage,4,PAGE_EXECUTE_READWRITE,&oldProt);
	*(PDWORD)dwHookStorage = (DWORD)dwHookFunction;
	//VirtualProtect((PVOID)dwHookStorage,4,oldProt,&oldProt2);

	// Install the Jmp code.
	//VirtualProtect((PVOID)dwInstallAddress,iJmpCodeSize,PAGE_EXECUTE_READWRITE,&oldProt);
	memcpy((PVOID)dwInstallAddress,pbyteJmpCode,iJmpCodeSize);
	//VirtualProtect((PVOID)dwInstallAddress,iJmpCodeSize,oldProt,&oldProt2);
}

//-----------------------------------------------------------

void InstallCallHook(DWORD dwInstallAddress, DWORD dwHookFunction, BYTE byteJumpCode = 0xE8)
{
	//DWORD oldProt, oldProt2;
	DWORD disp = dwHookFunction - (dwInstallAddress + 5);

	//VirtualProtect((LPVOID)dwInstallAddress,5,PAGE_EXECUTE_READWRITE,&oldProt);
	*(PBYTE)(dwInstallAddress) = byteJumpCode;
	*(PDWORD)(dwInstallAddress+1) = (DWORD)disp;
	//VirtualProtect((LPVOID)dwInstallAddress,5,oldProt,&oldProt2);
}

//-----------------------------------------------------------

void InstallGameAndGraphicsLoopHooks()
{
	//UnFuck(0x53EB13, 4);
	*(int *)0x53EB13 = dwGraphicsLoop - 0x53EB12 - 5; // relative addr

	/*InstallHook(0x58C246,(DWORD)GameProcessHook,
		0x53BED1,GameProcess_HookJmpCode,sizeof(GameProcess_HookJmpCode));*/

	InstallCallHook(0x58FC53,(DWORD)CHud_DrawRadar_Hook);
	InstallCallHook(0x58FBBF,(DWORD)CHud_DrawCrossHairs_Hook);
	InstallCallHook(0x53C104,(DWORD)CCamera_Process_Hook);
	InstallCallHook(0x53E981,(DWORD)Hook_4);

	// For fixing fogging issues (needed for both debug and net)
	InstallMethodHook(0x86D1B0,(DWORD)CPed_Render_Hook); // This is PlayerPed
	InstallMethodHook(0x86C0F0,(DWORD)CPed_Render_Hook);
	InstallMethodHook(0x86C168,(DWORD)CPed_Render_Hook);
	InstallMethodHook(0x86C248,(DWORD)CPed_Render_Hook);
	InstallMethodHook(0x86C3A0,(DWORD)CPed_Render_Hook);

	/*
	InstallHook(0x5EFFE0,(DWORD)CPed_Say_Hook,
		0x5EFFD8,PedSay_HookJmpCode,sizeof(PedSay_HookJmpCode));*/

	InstallCallHook(0x53E930,(DWORD)FrameUpdate_Hook);
}

//-----------------------------------------------------------

void GameInstallHooks()
{
	//InstallHook(ADDR_RENDER2DSTUFF,(DWORD)GraphicsLoopHook,
		//ADDR_RENDER2DSTUFF_STORAGE,GraphicsLoop_HookJmpCode,
		//sizeof(GraphicsLoop_HookJmpCode));
	// Above code replaced by a new method which should avoid
	// stack corruption of the return value

	InstallGameAndGraphicsLoopHooks();

	InstallHook(0x4D3AA0, (DWORD)Hook_7, 0x4D3934, Hook_7_HookJmpCode, sizeof(Hook_7_HookJmpCode));
	InstallHook(0x4D4610, (DWORD)Hook_8, 0x4D4609, Hook_8_HookJmpCode, sizeof(Hook_8_HookJmpCode));

	InstallMethodHook(0x86D190,(DWORD)Hook_9);
	InstallMethodHook(0x86C0D0,(DWORD)Hook_10);
	InstallMethodHook(0x86D744,(DWORD)Hook_11);
	InstallCallHook(0x7330A2,(DWORD)CPed_GetWeaponSkillIndex_Hook);
	InstallMethodHook(0x86D194,(DWORD)CPlayerPed_ProcessCollision_Hook);

	InstallCallHook(0x5689FD,(DWORD)Hook_14);

	InstallCallHook(0x53EA03,(DWORD)Hook_15);

	InstallMethodHook(0x871148,(DWORD)Hook_16);
	InstallMethodHook(0x8721C8,(DWORD)Hook_16);
	InstallMethodHook(0x871388,(DWORD)Hook_16);
	InstallMethodHook(0x871970,(DWORD)Hook_16);
	InstallMethodHook(0x8716A8,(DWORD)Hook_16);
	InstallMethodHook(0x871550,(DWORD)Hook_16);
	InstallMethodHook(0x871800,(DWORD)Hook_16);
	InstallMethodHook(0x871B10,(DWORD)Hook_16);
	InstallMethodHook(0x872398,(DWORD)Hook_16);
	InstallMethodHook(0x871C50,(DWORD)Hook_16);

	// Vehicle Horn Fix for PlayerPeds.
	InstallCallHook(0x501B1D,(DWORD)VehicleHorn_Hook);
	InstallCallHook(0x501B42,(DWORD)VehicleHorn_Hook);
	InstallCallHook(0x501FC2,(DWORD)VehicleHorn_Hook);
	InstallCallHook(0x502067,(DWORD)VehicleHorn_Hook);
	InstallCallHook(0x5021AE,(DWORD)VehicleHorn_Hook);

	// Radar and map hooks for gang zones
	InstallCallHook(0x5869BF,(DWORD)ZoneOverlay_Hook);
	InstallCallHook(0x5759E4,(DWORD)ZoneOverlay_Hook);

	InstallCallHook(0x609A4E,(DWORD)PlayerWalk_Hook);
	InstallCallHook(0x4579C6,(DWORD)PickUpPickup_Hook, 0xE9);

	InstallCallHook(0x73C252,(DWORD)CWeapon_FireCamera_Hook,0xE9);

	// Hook the call to CCamera::CamShake when called for sniper fire
	InstallCallHook(0x73ACE2,(DWORD)CCameraCamShake_Sniper_Hook);

	// Hook the train derailment code
	InstallCallHook(0x6F8CF8,(DWORD)CTrain_ProcessControl_Derailment);

	InstallHook(0x6402F0, (DWORD)Hook_24, 0x6919BB, Hook_24_HookJmpCode, sizeof(Hook_24_HookJmpCode));
	InstallHook(0x63B8C0, (DWORD)Hook_25, 0x63B8BA, Hook_25_HookJmpCode, sizeof(Hook_25_HookJmpCode));
	
	InstallHook(0x438576,(DWORD)CheatProcessorHook,
		0x4385AA,CheatProcessHook_JmpCode,sizeof(CheatProcessHook_JmpCode));
	
	InstallHook(0x584770,(DWORD)RadarTranslateColor,0x584A79,
		RadarTranslateColor_HookJmpCode,sizeof(RadarTranslateColor_HookJmpCode));
	
	InstallHook(0x53C900,(DWORD)CGameShutdownHook,0x53C8F1,
		CGameShutdown_HookJmpCode,sizeof(CGameShutdown_HookJmpCode));

	InstallHook(0x4B5AC0, (DWORD)Hook_29, 0x4B5ABC, Hook_29_HookJmpCode, sizeof(Hook_29_HookJmpCode));

	// Fix for 0x004D41C5 crash
	InstallCallHook(0x4D41C0,(DWORD)AnimCrashFixHook,0xE9);

	InstallCallHook(0x4E7427,(DWORD)CrimeReport_Hook);

	// Fix for crash when the player who threw the satchel died
	InstallHook(0x738F3A, (DWORD)CProjectile_Update_Hook, 0x738B1B, CProjectileInfo_Update_HookJmpCode, sizeof(CProjectileInfo_Update_HookJmpCode));
	// Fix for all satchels blowing up when someone activated their satchel
	InstallHook(0x738877, (DWORD)CWeapon__Satchel__Activate_Hook, 0x73885B, CWeapon__Satchel__Activate_HookJmpCode, sizeof(CWeapon__Satchel__Activate_HookJmpCode));
	
	InstallHook(0x6A0050, (DWORD)GetText_Hook, 0x6A0043, GetText_HookJmpCode, sizeof(GetText_HookJmpCode));

	InstallCallHook(0x6FDED6,(DWORD)Hook_35);

	if (iGtaVersion == GTASA_VERSION_USA10)
	{
		InstallHook(0x7FB020, (DWORD)Hook_36, 0x59C721, Hook_36_HookJmpCode, sizeof(Hook_36_HookJmpCode));
		dword_1018F56C = 0x7FB026;
	}
	else
	{
		InstallHook(0x7FB060, (DWORD)Hook_36, 0x59C721, Hook_36_HookJmpCode, sizeof(Hook_36_HookJmpCode));
		dword_1018F56C = 0x7FB066;
	}

	InstallCallHook(0x6D0E7E,(DWORD)UpdateVehicleTexture_Hook);

	InstallMethodHook(0x866FA8,(DWORD)Hook_38);

	InstallCallHook(0x586C0A,(DWORD)GetPlayerVelocity_Hook);

	InstallHook(0x538090, (DWORD)Hook_40, 0x538084, Hook_40_HookJmpCode, sizeof(Hook_40_HookJmpCode));

	InstallCallHook(0x718599,(DWORD)UpdateEscalator_Hook);

	InstallMethodHook(0x866F7C,(DWORD)CreateRwObject_Hook);

	InstallMethodHook(0x866F80,(DWORD)DeleteRwObject_Hook);
	InstallMethodHook(0x8585E8,(DWORD)DeleteRwObject_Hook);

	InstallMethodHook(0x871218,(DWORD)Hook_44);
	InstallMethodHook(0x871778,(DWORD)Hook_44);
	InstallMethodHook(0x8718D0,(DWORD)Hook_44);
	InstallMethodHook(0x871A40,(DWORD)Hook_44);
	InstallMethodHook(0x871BE0,(DWORD)Hook_44);

	InstallCallHook(0x5648D3,(DWORD)Hook_45);

	InstallCallHook(0x53DFDD,(DWORD)Hook_46);

	InstallCallHook(0x53E019,(DWORD)Hook_47);

	InstallHook(0x5534B0, (DWORD)Hook_48, 0x5534A6, Hook_48_HookJmpCode, sizeof(Hook_48_HookJmpCode));

	InstallCallHook(0x5342F9,(DWORD)RoadsignCrashFixHook);

	InstallHook(0x4B35A0,(DWORD)KillEvent_Hook,0x4B3433,
		KillEvent_HookJmpCode,sizeof(KillEvent_HookJmpCode));

	InstallCallHook(0x41B02E,(DWORD)Hook_51);

	InstallCallHook(0x41AF80,(DWORD)InVehicleCameraCheck_Hook);
	InstallCallHook(0x41AB78,(DWORD)Hook_53);

	InstallMethodHook(0x871178,(DWORD)Hook_54);
	InstallMethodHook(0x8716D8,(DWORD)Hook_54);
	InstallMethodHook(0x8719A0,(DWORD)Hook_54);
	InstallMethodHook(0x871B40,(DWORD)Hook_54);
	InstallMethodHook(0x8713B8,(DWORD)Hook_55);
	InstallMethodHook(0x871580,(DWORD)Hook_55);
	InstallMethodHook(0x871830,(DWORD)Hook_56);
	InstallMethodHook(0x8721F8,(DWORD)Hook_57);

	InstallCallHook(0x6E0954,(DWORD)Hook_58);
	InstallCallHook(0x6B2BCB,(DWORD)Hook_58);
	InstallCallHook(0x4F77DA,(DWORD)Hook_58);

	InstallMethodHook(0x872A74,(DWORD)Hook_59);

	InstallHook(0x5674E0, (DWORD)Hook_60, 0x5674DB, Hook_60_HookJmpCode, sizeof(Hook_60_HookJmpCode));
}

//-----------------------------------------------------------

void InstallModelInfoHooks()
{

}

//-----------------------------------------------------------
