#pragma once
#include <string>
#include <windows.h>
#include "utils.h"
class ProcessMemoryManager
{
public:
	static unsigned char* ReadMemory(DWORD dataOffset, const int length, bool offsetIsPointer, int additionalOffset = 0) 
	{
		HANDLE hProcess = GetBaseAddressByName();
		SIZE_T bytesRead = 0;
		unsigned char* data = new unsigned char[length];
		DWORD offset = offsetIsPointer ? GetOffsetFromPointer(hProcess, dataOffset) : dataOffset;
		ReadProcessMemory(hProcess, (void*)(offset + additionalOffset), data, length, &bytesRead);
		return data;
	}
	static void WriteMemory(DWORD dataOffset, unsigned char* data, bool offsetIsPointer, int length, int additionalOffset = 0) 
	{
		HANDLE hProcess = GetModuleHandle(0);
		SIZE_T bytesWritten = 0;
		UINT_PTR offset = offsetIsPointer ? GetOffsetFromPointer(hProcess, dataOffset) : dataOffset;
		WriteProcessMemory(hProcess, (void*)(offset + additionalOffset), data, length, &bytesWritten);
	}
	static DWORD GetOffsetFromPointer(HANDLE hmod, DWORD pointerOffset)
	{
		SIZE_T bytesRead = 0;
		unsigned char bufferAddress[4];

		DWORD ptr = (DWORD)hmod + pointerOffset;
		ReadProcessMemory(hmod, (void*)ptr, bufferAddress, 4, &bytesRead);

		DWORD offset = bitsToInt(offset, bufferAddress);

		return offset;
	}

	static HANDLE GetBaseAddressByName()
	{

		DWORD aProcesses[1024];
		DWORD cbNeeded;
		DWORD cProcesses;

		// Get the list of process identifiers.
		if (!EnumProcesses(aProcesses, sizeof(aProcesses), &cbNeeded))
			exit(0);

		// Calculate how many process identifiers were returned.
		cProcesses = cbNeeded / sizeof(DWORD);

		// Check the names of all the processess (Case insensitive)
		for (int i = 0; i < cProcesses; i++) {
			DWORD processId = aProcesses[i];
			TCHAR* processName = TEXT("GuiltyGearXrd.exe");


			TCHAR szProcessName[MAX_PATH] = TEXT("<unknown>");

			HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION |
				PROCESS_VM_READ | PROCESS_VM_WRITE | PROCESS_VM_OPERATION,
				FALSE, processId);

			if (NULL != hProcess)
			{
				HMODULE hMod;
				DWORD cbNeeded;

				if (EnumProcessModulesEx(hProcess, &hMod, sizeof(hMod),
					&cbNeeded, LIST_MODULES_32BIT | LIST_MODULES_64BIT))
				{
					GetModuleBaseName(hProcess, hMod, szProcessName,
						sizeof(szProcessName) / sizeof(TCHAR));
					if (!_tcsicmp(processName, szProcessName)) {
						return hProcess;
					}
					else CloseHandle(hProcess);
				}
			}
		}
	}

private:
};