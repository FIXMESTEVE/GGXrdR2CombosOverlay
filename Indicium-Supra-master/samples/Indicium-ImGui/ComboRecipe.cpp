#include "ComboRecipe.h"
#include <cstdint>
#include <vector>
#include <sstream>

ComboRecipe::ComboRecipe(unsigned char * comboData, int slotNr)
{
	int32_t head_addr = *reinterpret_cast<int32_t*>(comboData + slotNr * SLOT_DATA_SIZE);
	chrCode = (CharacterCode)head_addr;
	unsigned char* first = comboData + slotNr * SLOT_DATA_SIZE + SLOT_CHARCODE_SIZE;
	size_t length = SLOT_DATA_SIZE - SLOT_CHARCODE_SIZE;
	std::memcpy(first, MoveData, length);
}

ComboRecipe::~ComboRecipe()
{
	delete MoveData;
}

unsigned char * ComboRecipe::ToRecipeData()
{
	unsigned char * ret = new unsigned char[SLOT_DATA_SIZE + SLOT_CHARCODE_SIZE];

	unsigned char arrayOfByte[4];
	for (int i = 0; i < 4; i++)
	{
		arrayOfByte[3 - i] = ((int)chrCode >> (i * 8));
	}

	std::stringstream ss;
	ss << arrayOfByte << MoveData;
	ss >> ret;

	return ret;
}
