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

#include <imgui.h>
#include "imgui_main.h"
#include "panels/menubar/menubar.h"
#include "panels/stickerinspect/stickerinspect.h"

namespace GUI
{

void DrawMainWindow()
{
	if (g_GUICtx.m_WindowStates.m_bDemoWindow)
		ImGui::ShowDemoWindow(&g_GUICtx.m_WindowStates.m_bDemoWindow);

	if (g_GUICtx.m_WindowStates.m_bStickerInspect)
		StickerInspect::Draw(&g_GUICtx.m_WindowStates.m_bStickerInspect);

	MenuBar::Draw();
}

} // namespace GUI