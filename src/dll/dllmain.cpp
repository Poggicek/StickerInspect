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

#include "dllmain.h"
#include "eiface.h"
#include "thread"
#include "gui/imgui_main.h"
#include "modules.h"
#include "interfaces.h"
#include "utils/common.h"
#include "timer.h"
#include <filesystem.h>
#include <KeyValues.h>
#include <filesystem>
#include <fstream>
#include <entity2/entitysystem.h>
#include <iserver.h>
#include <safetyhook.hpp>

class CAttributeList;

CAttributeListSetOrAddAttributeValueByName g_pSetAttribute;
static SafetyHookVmt g_ServerVMT;
static SafetyHookVm g_gameFrameHook;

CGameEntitySystem* GameEntitySystem()
{
#ifdef WIN32
	static int offset = 88;
#else
	static int offset = 80;
#endif
	return *reinterpret_cast<CGameEntitySystem**>((uintptr_t)(Interfaces::gameResourceServiceServer)+offset);
}

CGlobalVars* GetGameGlobals()
{
	INetworkGameServer* server = Interfaces::networkServerService->GetIGameServer();

	if (!server)
		return nullptr;

	return server->GetGlobals();
}

void InitModulesAndInterfaces()
{
	// Modules
	Modules::fileSystem = std::make_unique<CModule>(ROOTBIN, "filesystem_stdio");
	Modules::engine = std::make_unique<CModule>(ROOTBIN, "engine2");
	Modules::schemaSystem = std::make_unique<CModule>(ROOTBIN, "schemasystem");
	Modules::server = std::make_unique<CModule>(GAMEBIN, "server");

	// Interfaces
	Interfaces::fileSystem = Modules::fileSystem->FindInterface<IFileSystem*>(FILESYSTEM_INTERFACE_VERSION);
	Interfaces::engineServer = Modules::engine->FindInterface<IVEngineServer2*>(SOURCE2ENGINETOSERVER_INTERFACE_VERSION);
	Interfaces::gameResourceServiceServer = Modules::engine->FindInterface<IGameResourceService*>(GAMERESOURCESERVICESERVER_INTERFACE_VERSION);
	Interfaces::networkServerService = Modules::engine->FindInterface<INetworkServerService*>(NETWORKSERVERSERVICE_INTERFACE_VERSION);
	Interfaces::schemaSystem2 = Modules::schemaSystem->FindInterface<CSchemaSystem*>(SCHEMASYSTEM_INTERFACE_VERSION);
	Interfaces::server = Modules::server->FindInterface<IServerGameDLL*>(INTERFACEVERSION_SERVERGAMEDLL);

	const byte sig[] = "\x40\x53\x41\x56\x41\x57\x48\x81\xEC\x90\x00\x00\x00\x0F\x29\x74\x24\x70";
	int err;
	g_pSetAttribute = (CAttributeListSetOrAddAttributeValueByName)Modules::server->FindSignature((byte*)sig, sizeof(sig) - 1, err);
}

void OnGameFrameHook(IServerGameDLL* server, bool simulating, bool bFirstTick, bool bLastTick)
{
	std::erase_if(g_vecNextTick, [](const auto& fn) {
		fn();
		return true;
	});

	g_gameFrameHook.thiscall(server, simulating, bFirstTick, bLastTick);
}

void InitHooks()
{
	g_ServerVMT = safetyhook::create_vmt(Interfaces::server);
	// 19 is GameFrame vtable index
	auto gameFrameHook = g_ServerVMT.hook_method(19, &OnGameFrameHook);

	if (gameFrameHook)
		g_gameFrameHook = std::move(*gameFrameHook);
}

unsigned long WINAPI initial_thread(void* reserved)
{
	AllocConsole();
	freopen_s((FILE**)stdout, "CONOUT$", "w", stdout);
	freopen_s((FILE**)stdin, "CONIN$", "r", stdin);
	InitModulesAndInterfaces();
	InitHooks();

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