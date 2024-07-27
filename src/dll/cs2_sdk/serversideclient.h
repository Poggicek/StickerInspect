#pragma once

#include "playerslot.h"
#include "steam/steamclientpublic.h"
#include "utlstring.h"
#include "inetchannel.h"

class INetworkMessageProcessingPreFilterCustom
{
public:
	virtual void FilterMessage() = 0;

	INetChannel* GetNetChannel() const { return m_NetChannel; }
	CPlayerSlot GetPlayerSlot() const { return m_nClientSlot; }
	CPlayerUserId GetUserID() const { return m_UserID; }
	int GetSignonState() const { return m_nSignonState; }
	CSteamID* GetClientSteamID() const { return (CSteamID*)&m_SteamID; }
	const char* GetClientName() const { return m_Name.Get(); }
	netadr_t* GetRemoteAddress() const { return (netadr_t*)&m_NetAdr0; }

private:
	[[maybe_unused]] char pad1[0x30];
#ifdef __linux__
	[[maybe_unused]] char pad2[0x10];
#endif
	CUtlString m_Name;			 // 64 | 80
	CPlayerSlot m_nClientSlot;	 // 72 | 88
	CEntityIndex m_nEntityIndex; // 76 | 92
	[[maybe_unused]] char pad3[0x8];
	INetChannel* m_NetChannel; // 88 | 104
	[[maybe_unused]] char pad4[0x4];
	int32 m_nSignonState; // 100 | 116
	[[maybe_unused]] char pad5[0x38];
	bool m_bFakePlayer; // 160 | 176
	[[maybe_unused]] char pad6[0x7];
	CPlayerUserId m_UserID; // 168 | 184
	[[maybe_unused]] char pad7[0x1];
	CSteamID m_SteamID; // 171 | 187
	[[maybe_unused]] char pad8[0x19];
	netadr_t m_NetAdr0; // 204 | 220
	[[maybe_unused]] char pad9[0x14];
	netadr_t m_NetAdr1; // 236 | 252
	[[maybe_unused]] char pad10[0x22];
	bool m_bIsHLTV; // 282 | 298
	[[maybe_unused]] char pad11[0x19];
	int m_nDeltaTick; // 308 | 324
	[[maybe_unused]] char pad12[0x882];
	bool m_bFullyAuthenticated; // 2490 | 2506
};