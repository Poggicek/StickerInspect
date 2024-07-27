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

#include "menubar.h"
#include "gui/imgui_main.h"
#include <imgui.h>

void GUI::MenuBar::Draw()
{
	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("Tools"))
		{
			ImGui::MenuItem("Demo Window", nullptr, &g_GUICtx.m_WindowStates.m_bDemoWindow);
			ImGui::MenuItem("Sticker Inspect", nullptr, &g_GUICtx.m_WindowStates.m_bStickerInspect);
			ImGui::EndMenu();
		}
		ImGui::EndMainMenuBar();
	}
}