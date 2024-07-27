/**
 * =============================================================================
 * StickerInspect
 * Copyright (C) 2024 Poggu
 *
 * DLL-Injector
 * Copyright (C) 2021-2024 adamhlt
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

#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>
#include <TlHelp32.h>
#include <iostream>
#include <filesystem>

DWORD GetProcessByName(const char* lpProcessName)
{
	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
	if (hSnapshot == INVALID_HANDLE_VALUE) return false;

	//Used to store the process info in the loop
	PROCESSENTRY32 ProcEntry;
	ProcEntry.dwSize = sizeof(PROCESSENTRY32);

	//Get the first process
	if (Process32First(hSnapshot, &ProcEntry)) {
		do
		{
			//If the found process name is equal to the on we're searching for
			if (!strcmp(ProcEntry.szExeFile, lpProcessName))
			{
				CloseHandle(hSnapshot);
				//Return the processID of the found process
				//TODO: return a list of found processes instead
				return ProcEntry.th32ProcessID;
			}
		} while (Process32Next(hSnapshot, &ProcEntry)); //Get the next process
	}

	CloseHandle(hSnapshot);
    return -1;
}

class PauseWrapper
{
public:
	~PauseWrapper()
	{
		system("pause");
	}
};

int main()
{
	PauseWrapper pauseWrapper;

	const DWORD dwProcessID = GetProcessByName("cs2.exe");
	if (dwProcessID == (DWORD)-1)
	{
		printf("CS2 is not running\n");
		return -1;
	}

	char lpFullDLLPath[MAX_PATH];
	const DWORD dwFullPathResult = GetFullPathNameA("StickerInspect.dll", MAX_PATH, lpFullDLLPath, nullptr);
	if (dwFullPathResult == 0)
	{
		printf("An error is occured when trying to get the full path of the DLL.\n");
		return -1;
	}

	if(!std::filesystem::exists(lpFullDLLPath))
	{
		printf("Failed to find StickerInspect.dll, make sure the two files are next to each other.\n");
		return -1;
	}

	const HANDLE hTargetProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwProcessID);
	if (hTargetProcess == INVALID_HANDLE_VALUE)
	{
		printf("An error is occured when trying to open the target process.\n");
		return -1;
	}

	const LPVOID lpPathAddress = VirtualAllocEx(hTargetProcess, nullptr, lstrlenA(lpFullDLLPath) + 1, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
	if (lpPathAddress == nullptr)
	{
		printf("An error is occured when trying to allocate memory in the target process.\n");
		return -1;
	}

	const DWORD dwWriteResult = WriteProcessMemory(hTargetProcess, lpPathAddress, lpFullDLLPath, lstrlenA(lpFullDLLPath) + 1, nullptr);
	if (dwWriteResult == 0)
	{
		printf("An error is occured when trying to write the DLL path in the target process.\n");
		return -1;
	}

	const HMODULE hModule = GetModuleHandleA("kernel32.dll");
	if (hModule == INVALID_HANDLE_VALUE || hModule == nullptr)
		return -1;

	const FARPROC lpFunctionAddress = GetProcAddress(hModule, "LoadLibraryA");
	if (lpFunctionAddress == nullptr)
	{
		printf("An error is occured when trying to get \"LoadLibraryA\" address.\n");
		return -1;
	}

	const HANDLE hThreadCreationResult = CreateRemoteThread(hTargetProcess, nullptr, 0, (LPTHREAD_START_ROUTINE)lpFunctionAddress, lpPathAddress, 0, nullptr);
	if (hThreadCreationResult == INVALID_HANDLE_VALUE)
	{
		printf("An error is occured when trying to create the thread in the target process.\n");
		return -1;
	}

	printf("DLL Injected !\n");
}