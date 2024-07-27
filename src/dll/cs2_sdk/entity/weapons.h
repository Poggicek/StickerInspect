/**
 * =============================================================================
 * CS2Fixes
 * Copyright (C) 2023-2024 Source2ZE
 * =============================================================================
 *
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License, version 3.0, as published by the
 * Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include "../schema.h"
#include "ehandle.h"
#include "cbaseentity.h"
#include "cbaseplayerpawn.h"

class CEntitySubclassVDataBase
{
public:
	DECLARE_SCHEMA_CLASS(CEntitySubclassVDataBase)
};


enum gear_slot_t : uint32_t
{
	GEAR_SLOT_INVALID = 0xffffffff,
	GEAR_SLOT_RIFLE = 0x0,
	GEAR_SLOT_PISTOL = 0x1,
	GEAR_SLOT_KNIFE = 0x2,
	GEAR_SLOT_GRENADES = 0x3,
	GEAR_SLOT_C4 = 0x4,
	GEAR_SLOT_RESERVED_SLOT6 = 0x5,
	GEAR_SLOT_RESERVED_SLOT7 = 0x6,
	GEAR_SLOT_RESERVED_SLOT8 = 0x7,
	GEAR_SLOT_RESERVED_SLOT9 = 0x8,
	GEAR_SLOT_RESERVED_SLOT10 = 0x9,
	GEAR_SLOT_RESERVED_SLOT11 = 0xa,
	GEAR_SLOT_BOOSTS = 0xb,
	GEAR_SLOT_UTILITY = 0xc,
	GEAR_SLOT_COUNT = 0xd,
	GEAR_SLOT_FIRST = 0x0,
	GEAR_SLOT_LAST = 0xc,
};

class CEconItemAttribute
{
public:
	virtual void unk() {};
private:
	[[maybe_unused]] uint8_t __pad0000[0x28]; // 0x0
public:
	// MNetworkEnable
	uint16_t m_iAttributeDefinitionIndex = 0.0; // 0x30	
private:
	[[maybe_unused]] uint8_t __pad0032[0x2]; // 0x32
public:
	// MNetworkEnable
	// MNetworkAlias "m_iRawValue32"
	float m_flValue = 0.0; // 0x34	
	// MNetworkEnable
	float m_flInitialValue = 0.0; // 0x38	
	// MNetworkEnable
	int32_t m_nRefundableCurrency = 0; // 0x3c	
	// MNetworkEnable
	bool m_bSetBonus = false; // 0x40	
	char pad[0x7];
};

class CAttributeList
{
	DECLARE_SCHEMA_CLASS_INLINE(CAttributeList);
public:
	SCHEMA_FIELD_POINTER(CUtlVector<CEconItemAttribute>, m_Attributes)
};

class CEconItemView
{
public:
	DECLARE_SCHEMA_CLASS_INLINE(CEconItemView);

		SCHEMA_FIELD(uint16_t, m_iItemDefinitionIndex)
		SCHEMA_FIELD(bool, m_bInitialized)
		SCHEMA_FIELD(uint32_t, m_iAccountID)
		SCHEMA_FIELD(uint64_t, m_iItemID)
		SCHEMA_FIELD(uint32_t, m_iItemIDHigh)
		SCHEMA_FIELD(uint32_t, m_iItemIDLow)
		SCHEMA_FIELD(int32_t, m_iEntityQuality)
		SCHEMA_FIELD(CAttributeList, m_NetworkedDynamicAttributes)
		SCHEMA_FIELD_POINTER(char, m_szCustomName)
};


class CAttributeContainer
{
public:
	DECLARE_SCHEMA_CLASS_INLINE(CAttributeContainer);

	SCHEMA_FIELD(CEconItemView, m_Item)
};

class CEconEntity : public Z_CBaseEntity
{
public:
	DECLARE_SCHEMA_CLASS(CEconEntity)

	SCHEMA_FIELD(CAttributeContainer, m_AttributeManager)
	SCHEMA_FIELD(CHandle<CBasePlayerPawn>, m_hOldProvidee)
};

class CBasePlayerWeaponVData : public CEntitySubclassVDataBase
{
public:
	DECLARE_SCHEMA_CLASS(CBasePlayerWeaponVData)
	SCHEMA_FIELD(int, m_iMaxClip1)
};

class CCSWeaponBaseVData : public CBasePlayerWeaponVData
{
public:
	DECLARE_SCHEMA_CLASS(CCSWeaponBaseVData)

	SCHEMA_FIELD(gear_slot_t, m_GearSlot)
		SCHEMA_FIELD(int, m_nPrice)
		SCHEMA_FIELD(int, m_nPrimaryReserveAmmoMax);
};


class CBasePlayerWeapon : public CEconEntity
{
public:
	DECLARE_SCHEMA_CLASS(CBasePlayerWeapon)
};