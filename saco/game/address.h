//----------------------------------------------------------
//
// SA:MP Multiplayer Modification For GTA:SA
// Copyright 2004-2005 SA:MP team
//
// Version: $Id: address.h,v 1.5 2006/03/20 17:44:19 kyeman Exp $
//
//----------------------------------------------------------

#pragma once

//-----------------------------------------------------------

#define ADDR_HWND								0xC97C1C
#define ADDR_ID3D9DEVICE						0xC97C28
#define ADDR_ID3D9								0xC97C20
#define ADDR_KEYSTATES							0xB73458

#define ADDR_MENU								0xBA67A4
#define ADDR_MENU2								0xBA67A5
#define ADDR_MENU3								0xBA67A6

#define ADDR_STARTGAME							0xBA677B
#define ADDR_GAME_STARTED						0xBA6831
#define ADDR_ENTRY								0xC8D4C0

#define ADDR_BYPASS_VIDS_USA10					0x747483
#define ADDR_BYPASS_VIDS_EU10					0x7474D3

#define ADDR_RENDERWARE_GETD3D_USA10			0x7F9D50
#define ADDR_RENDERWARE_GETD3D_EU10				0x7F9D90

#define ADDR_ENABLE_HUD							0xBA6769

// For loop hookings
#define ADDR_RENDER2DSTUFF						0x53E230
#define ADDR_RENDER2DSTUFF_STORAGE				0x53E22C

//#define ADDR_ID_FROM_ACTOR					0x451CF0
#define ADDR_ID_FROM_ACTOR						0x4442D0
#define ADDR_ACTOR_FROM_ID						0x404910  // Converts id to actor ptr
#define ADDR_PED_TABLE							0xB74490  // Contains ptr to actor/ped table

#define ADDR_ID_FROM_VEHICLE					0x42C4B0
#define ADDR_VEHICLE_FROM_ID					0x4048E0  // Converts id to vehicle ptr
#define ADDR_VEHICLE_TABLE						0xB74494  // Contains ptr to the vehicles table

#define ADDR_OBJECT_TABLE						0x94DBE0
#define ADDR_OBJECT_FROM_ID						0x451C30

#define ADDR_CAMERA								0xB6F99C

//-----------------------------------------------------------