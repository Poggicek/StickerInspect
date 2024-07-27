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
#include "winternl.h"

typedef ULONG(NTAPI* lpfNtQueryInformationProcess)(HANDLE, PROCESSINFOCLASS, PVOID, ULONG, PULONG);

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
		if(m_bPause)
			system("pause");
	}

	bool m_bPause = true;
};


// Checks command line in PEB to see if it contains -insecure and -tools
bool IsProcessInsecure(HANDLE hTargetProcess)
{
	const HMODULE NTDLL = GetModuleHandleA("ntdll.dll");
	lpfNtQueryInformationProcess NtQueryInformationProcess = (lpfNtQueryInformationProcess)GetProcAddress(NTDLL, "NtQueryInformationProcess");

	if (!NtQueryInformationProcess)
		return false;

	PROCESS_BASIC_INFORMATION pbi;
	DWORD dwLength;
	ULONG status = NtQueryInformationProcess(hTargetProcess, ProcessBasicInformation, &pbi, sizeof(PROCESS_BASIC_INFORMATION), &dwLength);

	if (status)
	{
		printf("An error occured when trying query PBI.\n");
		return false;
	}

	_PEB peb;
	size_t dwSize;

	if (!ReadProcessMemory(hTargetProcess, (void*)pbi.PebBaseAddress, &peb, sizeof(_PEB), &dwSize))
	{
		printf("An error occured when trying read PEB.\n");
		return false;
	}

	UNICODE_STRING commandLine;
	if (!ReadProcessMemory(hTargetProcess, &peb.ProcessParameters->CommandLine, &commandLine, sizeof(commandLine), nullptr))
	{
		printf("An error occured when reading command line.\n");
		return false;
	}

	auto buffer = new WCHAR[commandLine.MaximumLength];

	if (!buffer)
		return false;

	if (!ReadProcessMemory(hTargetProcess, commandLine.Buffer, buffer, commandLine.MaximumLength, nullptr))
	{
		printf("An error occured when reading command line buffer.\n");
		return false;
	}

	auto string = std::wstring(buffer);

	bool isInsecure = string.find(L" -insecure") != std::string::npos && string.find(L" -tools") != std::string::npos;

	delete buffer;
	return isInsecure;

}

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

	if (!IsProcessInsecure(hTargetProcess))
	{
		MessageBoxA(nullptr, "CS2 is not running with -tools and -insecure launch params!!!", "Error", MB_ICONERROR);
		printf("CS2 is not running with -tools and -insecure launch params!!!\n");
		CloseHandle(hTargetProcess);
		return -1;
	}

	const LPVOID lpPathAddress = VirtualAllocEx(hTargetProcess, nullptr, lstrlenA(lpFullDLLPath) + 1, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
	if (lpPathAddress == nullptr)
	{
		printf("An error occured when trying to allocate memory in the target process.\n");
		CloseHandle(hTargetProcess);
		return -1;
	}

	const DWORD dwWriteResult = WriteProcessMemory(hTargetProcess, lpPathAddress, lpFullDLLPath, lstrlenA(lpFullDLLPath) + 1, nullptr);
	if (dwWriteResult == 0)
	{
		printf("An error occured when trying to write the DLL path in the target process.\n");
		CloseHandle(hTargetProcess);
		return -1;
	}

	const HMODULE hModule = GetModuleHandleA("kernel32.dll");
	if (hModule == INVALID_HANDLE_VALUE || hModule == nullptr)
	{
		CloseHandle(hTargetProcess);
		return -1;
	}

	const FARPROC lpFunctionAddress = GetProcAddress(hModule, "LoadLibraryA");
	if (lpFunctionAddress == nullptr)
	{
		printf("An error occured when trying to get \"LoadLibraryA\" address.\n");
		CloseHandle(hTargetProcess);
		return -1;
	}

	const HANDLE hThreadCreationResult = CreateRemoteThread(hTargetProcess, nullptr, 0, (LPTHREAD_START_ROUTINE)lpFunctionAddress, lpPathAddress, 0, nullptr);
	if (hThreadCreationResult == INVALID_HANDLE_VALUE)
	{
		printf("An error occured when trying to create the thread in the target process.\n");
		CloseHandle(hTargetProcess);
		return -1;
	}

	printf("DLL Injected !\n");
	CloseHandle(hTargetProcess);
	pauseWrapper.m_bPause = false; // Don't leave console open if there were no errors.
}