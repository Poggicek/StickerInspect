/**
 * =============================================================================
 * StickerInspect
 * Copyright (C) 2024 Poggu
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

#include "stickerinspect.h"
#include "gui/imgui_main.h"
#include "interfaces.h"
#include <string>
#include <imgui.h>
#include <KeyValues.h>
#include <vector>
#include "cstrike15_gcmessages.pb.h"
#include "utils/crc32.h"
#include "interfaces.h"
#include <sstream>
#include <iomanip>
#include <eiface.h>
#include <entity2/entitysystem.h>
#include "cs2_sdk/entity/cbaseplayercontroller.h"
#include "cs2_sdk/entity/cbaseplayerpawn.h"
#include "cs2_sdk/entity/weapons.h"
#include "timer.h"
#include "dllmain.h"
#include "gui/gui.h"

namespace GUI::StickerInspect
{

class StickerOverride
{
public:
	uint32_t stickerId = -1;
	std::string stickerName = "<none>";
};

static int g_currentStickerIndex = -1;
static bool g_isDirty = true;
static std::vector<std::pair<std::string, int>> g_vecStickers;
static StickerOverride g_Stickers[5];
static std::map<int, bool> g_LegacyModelMap;
static bool g_bInspectOverride = false;
static char g_szInspectBuffer[1024] = "";
static std::mutex g_ToolMutex;
SafetyHookInline g_toolsInspectHook{};

void RefreshStickerList()
{
	g_vecStickers.clear();
	KeyValues* pKV = new KeyValues("items_game");
	KeyValues::AutoDelete autoDelete(pKV);

	bool res = pKV->LoadFromFile(Interfaces::fileSystem, "scripts/items/items_game_workshop.txt");
	if (res)
	{
		for (KeyValues* pKey = pKV->GetFirstSubKey(); pKey; pKey = pKey->GetNextKey())
		{
			// paint kits, legacy model check
			if (!V_stricmp(pKey->GetName(), "paint_kits"))
			{
				for (KeyValues* pPaintKey = pKey->GetFirstSubKey(); pPaintKey; pPaintKey = pPaintKey->GetNextKey())
				{
					g_LegacyModelMap[atoi(pPaintKey->GetName())] = pPaintKey->GetBool("use_legacy_model", false);
				}
			}

			// workshop sitckers
			if (V_stricmp(pKey->GetName(), "sticker_kits"))
				continue;

			for (KeyValues* pStickerKey = pKey->GetFirstSubKey(); pStickerKey; pStickerKey = pStickerKey->GetNextKey())
			{
				const char* itemName = pStickerKey->GetString("item_name", "");

				if (V_stricmp(itemName, "#CSGO_Workshop"))
					continue;

				g_vecStickers.push_back({ pStickerKey->GetString("name", ""), atoi(pStickerKey->GetName()) });
			}
		}
	}
}

void DrawStickerIndex(int stickerIndex)
{
	ImGui::PushID(stickerIndex);

	std::string comboName = "Sticker #" + std::to_string(stickerIndex + 1);
	if (ImGui::BeginCombo(comboName.c_str(), g_Stickers[stickerIndex].stickerName.c_str(), ImGuiComboFlags_HeightLarge))
	{
		if (ImGui::Selectable("<none>", g_Stickers[stickerIndex].stickerId == -1))
		{
			g_Stickers[stickerIndex].stickerId = -1;
			g_Stickers[stickerIndex].stickerName = "<none>";
		}

		for (int i = 0; i < g_vecStickers.size(); i++)
		{
			const auto& sticker = g_vecStickers[i];
			const bool selected = (g_Stickers[stickerIndex].stickerId == sticker.second);

			if (ImGui::Selectable(sticker.first.c_str(), selected))
			{
				g_Stickers[stickerIndex].stickerName = sticker.first;
				g_Stickers[stickerIndex].stickerId = sticker.second;
			}

			if (selected)
				ImGui::SetItemDefaultFocus();
		}

		ImGui::EndCombo();
	}

	ImGui::PopID();
}

std::vector<char> HexToBytes(const std::string& hex) {
	std::vector<char> bytes;

	for (unsigned int i = 0; i < hex.length(); i += 2) {
		std::string byteString = hex.substr(i, 2);
		char byte = (char)strtol(byteString.c_str(), NULL, 16);
		bytes.push_back(byte);
	}

	return bytes;
}

std::string bytes_to_hex_string(const std::vector<uint8_t>& input)
{
	static const char characters[] = "0123456789ABCDEF";

	// Zeroes out the buffer unnecessarily, can't be avoided for std::string.
	std::string ret(input.size() * 2, 0);

	// Hack... Against the rules but avoids copying the whole buffer.
	auto buf = const_cast<char*>(ret.data());

	for (const auto& oneInputByte : input)
	{
		*buf++ = characters[oneInputByte >> 4];
		*buf++ = characters[oneInputByte & 0x0F];
	}
	return ret;
}

void RunInspect(std::string& protoData)
{
	CEconItemPreviewDataBlock itemPreviewPB;

	auto byteArray = HexToBytes(protoData);
	bool success = itemPreviewPB.ParseFromArray(byteArray.data(), byteArray.size());

	if (!success)
		return;

	size_t i = 0;
	for (auto& sticker : *itemPreviewPB.mutable_stickers())
	{
		if (g_Stickers[i].stickerId != -1)
		{
			sticker.set_sticker_id(g_Stickers[i].stickerId);
		}
		i++;
	}

	std::vector<uint8_t> serializedProto;
	serializedProto.resize(itemPreviewPB.ByteSizeLong());
	itemPreviewPB.SerializeToArray(serializedProto.data(), serializedProto.capacity());

	serializedProto.insert(serializedProto.begin(), 0);

	crc32 crc;
	uint32_t crcValue = crc.update(serializedProto.data(), serializedProto.size());
	crcValue = (crcValue & 0xFFFF) ^ itemPreviewPB.ByteSize() * crcValue;
	crcValue &= 0xFFFFFFFF;

	serializedProto.push_back((crcValue & 0xff000000UL) >> 24);
	serializedProto.push_back((crcValue & 0x00ff0000UL) >> 16);
	serializedProto.push_back((crcValue & 0x0000ff00UL) >> 8);
	serializedProto.push_back((crcValue & 0x000000ffUL));

	std::string outCommand("csgo_econ_action_preview ");
	outCommand += bytes_to_hex_string(serializedProto);

	// Jump to main thread so that we won't run this from GUI thread
	NextTick([outCommand]() {
		Interfaces::engineServer->ServerCommand(outCommand.c_str());
	});
}

std::map<int, const char*> g_DefIndexMap {
	{1, "weapon_deagle"},
	{2, "weapon_elite"},
	{3, "weapon_fiveseven"},
	{4, "weapon_glock"},
	{7, "weapon_ak47"},
	{8, "weapon_aug"},
	{9, "weapon_awp"},
	{10, "weapon_famas"},
	{11, "weapon_g3sg1"},
	{13, "weapon_galilar"},
	{14, "weapon_m249"},
	{16, "weapon_m4a1"},
	{17, "weapon_mac10"},
	{19, "weapon_p90"},
	{23, "weapon_mp5sd"},
	{24, "weapon_ump45"},
	{25, "weapon_xm1014"},
	{26, "weapon_bizon"},
	{27, "weapon_mag7"},
	{28, "weapon_negev"},
	{29, "weapon_sawedoff"},
	{30, "weapon_tec9"},
	{31, "weapon_taser"},
	{32, "weapon_hkp2000"},
	{33, "weapon_mp7"},
	{34, "weapon_mp9"},
	{35, "weapon_nova"},
	{36, "weapon_p250"},
	{38, "weapon_scar20"},
	{39, "weapon_sg556"},
	{40, "weapon_ssg08"},
	{60, "weapon_m4a1_silencer"},
	{61, "weapon_usp_silencer"},
	{63, "weapon_cz75a"},
	{64, "weapon_revolver"},
};

static uint64_t g_iItemID = 65578;

void RunInGameInspect(std::string& protoData)
{
	CEconItemPreviewDataBlock itemPreviewPB;

	auto byteArray = HexToBytes(protoData);
	bool success = itemPreviewPB.ParseFromArray(byteArray.data(), byteArray.size());

	if (!success)
		return;

	// Jump to main thread so that we won't run this from GUI thread
	NextTick([itemPreviewPB]() {
		if (!g_pSetAttribute)
		{
			MessageBoxA(nullptr, "In Game Inspect is unavailable, check for updates.", "Error", MB_ICONERROR);
			return;
		}

		auto entitySystem = GameEntitySystem();
		auto globalVars = GetGameGlobals();

		// TODO: don't run this on every client, our client is prolly always first in list, or just check if the player is a bot or not.
		for (int i = 1; i < globalVars->maxClients; i++)
		{
			auto playerController = (CBasePlayerController*)entitySystem->GetEntityInstance(CEntityIndex(i));

			if (!playerController) continue;

			auto pawn = (CCSPlayerPawnBase*)playerController->m_hPawn().Get();

			if (!pawn) continue;

			auto itemServices = pawn->m_pItemServices();

			if (!itemServices) continue;

			const auto it = g_DefIndexMap.find(itemPreviewPB.defindex());
			if (it == g_DefIndexMap.end())
				continue;

			auto weapon = (CBasePlayerWeapon*)itemServices->GiveNamedItem(it->second);

			if (!weapon) continue;

			auto& econItem = weapon->m_AttributeManager().m_Item();

			auto itemID = g_iItemID++;
			econItem.m_iItemID = itemID;
			econItem.m_iItemIDHigh = itemID >> 32;
			econItem.m_iItemIDLow = itemID & 0xFFFFFFFF;
			econItem.m_iEntityQuality = 4;

			FOR_EACH_VEC(*(econItem.m_NetworkedDynamicAttributes().m_Attributes()), i)
			{
				auto& attribute = (*(econItem.m_NetworkedDynamicAttributes().m_Attributes()))[i];

				(*(weapon->m_AttributeManager().m_Item().m_NetworkedDynamicAttributes().m_Attributes())).Remove(i);
				i--;
			}

			size_t stickerIndex = 0;
			for (auto& sticker : itemPreviewPB.stickers())
			{
				int stickerID = g_Stickers[stickerIndex].stickerId != -1 ? g_Stickers[stickerIndex].stickerId : sticker.sticker_id();
				int schema = sticker.slot();
				std::string stickerPrefix = "sticker slot " + std::to_string(stickerIndex);

				g_pSetAttribute(econItem.m_NetworkedDynamicAttributes(), (stickerPrefix + " id").c_str(), *((float*)&stickerID));
				g_pSetAttribute(econItem.m_NetworkedDynamicAttributes(), (stickerPrefix + " schema").c_str(), *((float*)&schema));

				if (sticker.has_wear() && sticker.wear() != 0)
					g_pSetAttribute(econItem.m_NetworkedDynamicAttributes(), (stickerPrefix + " wear").c_str(), sticker.wear());

				if (sticker.has_rotation() && sticker.rotation() != 0)
					g_pSetAttribute(econItem.m_NetworkedDynamicAttributes(), (stickerPrefix + " rotation").c_str(), sticker.rotation());

				if(sticker.has_offset_x() && sticker.offset_x() != 0)
					g_pSetAttribute(econItem.m_NetworkedDynamicAttributes(), (stickerPrefix + " offset x").c_str(), sticker.offset_x());

				if (sticker.has_offset_y() && sticker.offset_y() != 0)
					g_pSetAttribute(econItem.m_NetworkedDynamicAttributes(), (stickerPrefix + " offset y").c_str(), sticker.offset_y());
				stickerIndex++;
			}

			g_pSetAttribute(econItem.m_NetworkedDynamicAttributes(), "set item texture prefab", itemPreviewPB.paintindex());
			g_pSetAttribute(econItem.m_NetworkedDynamicAttributes(), "set item texture wear", itemPreviewPB.paintwear());
			g_pSetAttribute(econItem.m_NetworkedDynamicAttributes(), "set item texture seed", itemPreviewPB.paintseed());

			bool isLegacyModel = false;

			const auto legacyMapIt = g_LegacyModelMap.find(itemPreviewPB.paintindex());
			if (legacyMapIt != g_LegacyModelMap.end())
				isLegacyModel = legacyMapIt->second;

			((CSkeletonInstance*)(weapon->m_CBodyComponent()->m_pSceneNode()))->m_modelState().m_MeshGroupMask = isLegacyModel ? 2 : 1;

			CCSPlayer_ViewModelServices* pViewModelServices =
				pawn->m_pViewModelServices();

			if (!pViewModelServices) continue;

			CBaseViewModel* pViewModel =
				pViewModelServices->m_hViewModel()[0].Get();

			if (!pViewModel) continue;

			((CSkeletonInstance*)(pViewModel->m_CBodyComponent()->m_pSceneNode()))->m_modelState().m_MeshGroupMask = isLegacyModel ? 2 : 1;
		}
	});
}

void ProcessInspect(bool inMenuButton, bool inGameButton)
{
	const std::lock_guard<std::mutex> lock(g_ToolMutex);

	CEconItemPreviewDataBlock itemPreviewPB;

	auto inspectLink = std::string(g_szInspectBuffer);

	if (inspectLink.find("csgo_econ_action_preview ") == 0)
	{
		auto protoData = inspectLink.substr(sizeof("csgo_econ_action_preview ") - 1 + 2); // +2 removes start nullbyte
		protoData = protoData.substr(0, protoData.size() - 8); //  -8 removes checksum at end
		printf("Proto data: %s\n", protoData.c_str());

		if (inMenuButton)
			RunInspect(protoData);
		else if (inGameButton)
			RunInGameInspect(protoData);
	};

}

void Draw(bool* isOpen)
{
	if (g_isDirty)
	{
		g_isDirty = false;
		RefreshStickerList();
	}

	ImGui::SetNextWindowSize(ImVec2(500, 400), ImGuiCond_FirstUseEver);
	ImGui::Begin("Sticker Inspect", isOpen);


	if (ImGui::Button("Refresh"))
		g_isDirty = true;

	for (int i = 0; i < 5; i++)
		DrawStickerIndex(i);

	{
		const std::lock_guard<std::mutex> lock(g_ToolMutex);
		ImGui::InputText("Inspect Link", g_szInspectBuffer, sizeof g_szInspectBuffer);
	}

	bool inMenuButton = ImGui::Button("Inspect In Menu");
	ImGui::SameLine();
	bool inGameButton = ImGui::Button("Inspect In Game");

	if (inMenuButton || inGameButton)
		ProcessInspect(inMenuButton, inGameButton);

	ImGui::Checkbox("Enable tool inspect hook", &g_bInspectOverride);
	ImGui::SameLine(); HelpMarker("Makes Inspect button inside workshop tools call our inspect instead");

	ImGui::End();
}

// THIS RUNS ON NON GUI THREAD
void OnToolInspect(void* qtObject)
{
	printf("Start inspect!\n");

	if (g_bInspectOverride)
		ProcessInspect(true, false);
	else
		g_toolsInspectHook.call(qtObject);
}

} // namespace GUI::StickerInspect