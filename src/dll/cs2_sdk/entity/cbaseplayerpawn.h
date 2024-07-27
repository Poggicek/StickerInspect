/**
 * =============================================================================
 * CS2Fixes
 * Copyright (C) 2023 Source2ZE
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

#include "cbaseentity.h"

class CPlayer_WeaponServices;
class CBasePlayerWeapon;

class CCSPlayer_ItemServices
{
public:
	DECLARE_SCHEMA_CLASS(CCSPlayer_ItemServices);

	virtual ~CCSPlayer_ItemServices() = 0;
private:
	virtual void unk_01() = 0;
	virtual void unk_02() = 0;
	virtual void unk_03() = 0;
	virtual void unk_04() = 0;
	virtual void unk_05() = 0;
	virtual void unk_06() = 0;
	virtual void unk_07() = 0;
	virtual void unk_08() = 0;
	virtual void unk_09() = 0;
	virtual void unk_10() = 0;
	virtual void unk_11() = 0;
	virtual void unk_12() = 0;
	virtual void unk_13() = 0;
	virtual void unk_14() = 0;
	virtual CBaseEntity* _GiveNamedItem(const char* pchName) = 0;
public:
	virtual bool         GiveNamedItemBool(const char* pchName) = 0;
	virtual CBaseEntity* GiveNamedItem(const char* pchName) = 0;
	// Recommended to use CCSPlayer_WeaponServices::DropWeapon instead (parameter is ignored here)
	virtual void         DropActiveWeapon(CBasePlayerWeapon* pWeapon) = 0;
	virtual void         StripPlayerWeapons(bool removeSuit) = 0;
};

class CBasePlayerPawn : public Z_CBaseEntity
{
public:
	DECLARE_SCHEMA_CLASS(CBasePlayerPawn);

	SCHEMA_FIELD(CPlayer_WeaponServices*, m_pWeaponServices)
	SCHEMA_FIELD(CCSPlayer_ItemServices*, m_pItemServices)
};



class CBaseViewModel : public Z_CBaseEntity
{
public:
	DECLARE_SCHEMA_CLASS(CBaseViewModel);

	SCHEMA_FIELD(CHandle<CBasePlayerWeapon>, m_hWeapon)
};

class CCSPlayer_ViewModelServices
{
public:
	DECLARE_SCHEMA_CLASS(CCSPlayer_ViewModelServices);

	SCHEMA_FIELD_POINTER(CHandle<CBaseViewModel>, m_hViewModel)
};

class CCSPlayerPawnBase : public CBasePlayerPawn
{
public:
	DECLARE_SCHEMA_CLASS(CCSPlayerPawnBase);

	SCHEMA_FIELD(CCSPlayer_ViewModelServices*, m_pViewModelServices)
};