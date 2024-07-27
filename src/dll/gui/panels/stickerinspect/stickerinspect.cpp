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
	if (ImGui::BeginCombo(comboName.c_str(), g_Stickers[stickerIndex].stickerName.c_str()))
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
	printf("status: %i %llu\n", success, byteArray.size());

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

	Interfaces::engineServer->ServerCommand(outCommand.c_str());
}

void RunInGameInspect(std::string& protoData)
{
	CEconItemPreviewDataBlock itemPreviewPB;

	auto byteArray = HexToBytes(protoData);
	bool success = itemPreviewPB.ParseFromArray(byteArray.data(), byteArray.size());
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
	{
		g_isDirty = true;
	}

	for (int i = 0; i < 5; i++)
	{
		DrawStickerIndex(i);
	}

	static char inspectBuffer[1024] = "";
	ImGui::InputText("Inspect Link", inspectBuffer, sizeof inspectBuffer);

	bool inMenuButton = ImGui::Button("Inspect In Menu");
	ImGui::SameLine();
	bool inGameButton = ImGui::Button("Inspect In Game");

	if (inMenuButton || inGameButton)
	{
		CEconItemPreviewDataBlock itemPreviewPB;

		auto inspectLink = std::string(inspectBuffer);

		if (inspectLink.find("csgo_econ_action_preview ") == 0)
		{
			auto protoData = inspectLink.substr(sizeof("csgo_econ_action_preview ") - 1 + 2); // +2 removes start nullbyte
			protoData = protoData.substr(0, protoData.size() - 8); //  -8 removes checksum at end
			printf("Proto data: %s\n", protoData.c_str());
			
			if (inMenuButton)
				RunInspect(protoData);
			else if(inGameButton)
				RunInGameInspect(protoData);
		};


		printf("Inspect url %s\n", inspectBuffer);
	}

	ImGui::End();
}

} // namespace GUI::StickerInspect