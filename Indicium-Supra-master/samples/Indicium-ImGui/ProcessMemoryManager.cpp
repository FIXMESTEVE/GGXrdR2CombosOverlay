#include "ProcessMemoryManager.h"
#include <windows.h>
#include <cstdio>
#include <tlhelp32.h>

unsigned char * ProcessMemoryManager::ReadMemory(int dataOffset, const int length, bool offsetIsPointer, int additionalOffset)
{
	HMODULE hmod = GetModuleHandle(0);
	SIZE_T bytesRead = 0;
	unsigned char* data = new unsigned char[length];
	int offset = offsetIsPointer ? GetOffsetFromPointer(hmod, dataOffset) : dataOffset;
	ReadProcessMemory(hmod, (LPVOID)(offset + additionalOffset), data, length, &bytesRead);
	return data;
}

void ProcessMemoryManager::WriteMemory(int dataOffset, unsigned char * data, bool offsetIsPointer, int length, int additionalOffset)
{
	HMODULE hmod = GetModuleHandle(0);
	SIZE_T bytesWritten = 0;
	int offset = offsetIsPointer ? GetOffsetFromPointer(hmod, dataOffset) : dataOffset;
	WriteProcessMemory(hmod, (LPVOID)(offset + additionalOffset), data, length, &bytesWritten);
}

int ProcessMemoryManager::GetOffsetFromPointer(HMODULE hmod, int pointerOffset)
{
	SIZE_T bytesRead = 0;
	unsigned char bufferAddress[4];

	DWORD ptr = (DWORD) hmod + pointerOffset;
	ReadProcessMemory(hmod, (LPVOID)ptr, bufferAddress, 4, &bytesRead);

	int32_t offset = *reinterpret_cast<int32_t*>(bufferAddress);
	return offset;
}

