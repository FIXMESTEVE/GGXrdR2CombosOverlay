#pragma once
#include "ComboRecipe.h"
#include <cstdint>
#include <vector>
#include <sstream>
#include "utils.h"

ComboRecipe::ComboRecipe(unsigned char * comboData, int slotNr)
{
	bool big_endian = is_big_endian();
	int chrCodeInt = bitsToInt(chrCodeInt, comboData + slotNr * SLOT_DATA_SIZE);
	chrCode = (CharacterCode)chrCodeInt;
	unsigned char* first = comboData + slotNr * SLOT_DATA_SIZE + SLOT_CHARCODE_SIZE;
	size_t length = SLOT_DATA_SIZE - SLOT_CHARCODE_SIZE;
	MoveData = new unsigned char[SLOT_DATA_SIZE - SLOT_CHARCODE_SIZE];
	std::memcpy(MoveData, first, length);
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
