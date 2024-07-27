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

#include "eiface.h"
#include "thread"
#include "gui/imgui_main.h"
#include <filesystem.h>
#include <KeyValues.h>
#include "modules.h"
#include "interfaces.h"
#include "utils/common.h"
#include <filesystem>
#include <fstream>

typedef void* (*CreateInterfaceFn)(const char* pName, int* pReturnCode);

void InitModulesAndInterfaces()
{
    // Modules
    Modules::fileSystem = std::make_unique<CModule>(ROOTBIN, "filesystem_stdio");
    Modules::engine = std::make_unique<CModule>(ROOTBIN, "engine2");
    
    // Interfaces
    Interfaces::fileSystem = Modules::fileSystem->FindInterface<IFileSystem*>(FILESYSTEM_INTERFACE_VERSION);
    Interfaces::engineServer = Modules::engine->FindInterface<IVEngineServer2*>(SOURCE2ENGINETOSERVER_INTERFACE_VERSION);
}


unsigned long WINAPI initial_thread(void* reserved)
{
    AllocConsole();
    freopen_s((FILE**)stdout, "CONOUT$", "w", stdout);
    freopen_s((FILE**)stdin, "CONIN$", "r", stdin);
    InitModulesAndInterfaces();

    auto gui_thread = std::thread([]() {
        GUI::InitializeGUI();
    });

    gui_thread.detach();

    return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule,
    DWORD  ul_reason_for_call,
    LPVOID lpReserved
)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
    {
        if (auto handle = CreateThread(nullptr, 0, initial_thread, hModule, 0, nullptr))
            CloseHandle(handle);
    }
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}