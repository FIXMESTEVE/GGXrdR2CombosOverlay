#pragma once
#include <string>
static class ProcessMemoryManager
{
public:
	static unsigned char* ReadMemory(int dataOffset, int length, bool offsetIsPointer, int additionalOffset = 0);
	static void WriteProcessMemory(int dataOffset, unsigned char* data, bool offsetIsPointer, int additionalOffset = 0);
	static int GetOffsetFromPointer(int pointerOffset);
	
private:
	const int _PROCESS_WM_READ = 0x0010;
	const int _PROCESS_VM_WRITE = 0x0020;
	const int _PROCESS_VM_OPERATION = 0x0008;
};