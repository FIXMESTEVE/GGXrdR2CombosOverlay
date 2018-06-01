#include "ProcessMemoryManager.h"
#include <windows.h>
#include <cstdio>
#include <tlhelp32.h>

unsigned char * ProcessMemoryManager::ReadMemory(int dataOffset, int length, bool offsetIsPointer, int additionalOffset)
{
	HMODULE hmod = GetModuleHandle(0);
	SIZE_T bytesRead = 0;
	unsigned char* data = new unsigned char[length];
	int offset = offsetIsPointer ? GetOffsetFromPointer(dataOffset) : dataOffset;
	ReadProcessMemory(hmod, (LPCVOID)(offset + additionalOffset), data, length, &bytesRead);
	return data;
}

void ProcessMemoryManager::WriteProcessMemory(int dataOffset, unsigned char * data, bool offsetIsPointer, int additionalOffset)
{
	//TODO
}

int ProcessMemoryManager::GetOffsetFromPointer(int pointerOffset)
{
	return 0;
}
