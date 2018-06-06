#pragma once
#include "ComboRecipe.h"
#include "ProcessMemoryManager.h"
#include <vector>
class ComboRecipeManager
{
public:
	static std::vector<ComboRecipe*> ReadComboRecipes() 
	{
		unsigned char* comboRecipeData = ProcessMemoryManager::ReadMemory(SLOT1_OFFSET_POINTER, ComboRecipe::SLOT_DATA_SIZE * 5, true);

		ComboRecipe* combo1 = new ComboRecipe(comboRecipeData, 0);
		ComboRecipe* combo2 = new ComboRecipe(comboRecipeData, 1);
		ComboRecipe* combo3 = new ComboRecipe(comboRecipeData, 2);
		ComboRecipe* combo4 = new ComboRecipe(comboRecipeData, 3);
		ComboRecipe* combo5 = new ComboRecipe(comboRecipeData, 4);

		//delete comboRecipeData;

		std::vector<ComboRecipe*> arr = { combo1, combo2, combo3, combo4, combo5 };

		return arr;
	}
	static ComboRecipe* ReadComboRecipe(int slotNr)
	{
		unsigned char* data = ProcessMemoryManager::ReadMemory(SLOT1_OFFSET_POINTER, ComboRecipe::SLOT_DATA_SIZE, true, slotNr * ComboRecipe::SLOT_DATA_SIZE);

		if (data != NULL)
		{
			ComboRecipe* c = new ComboRecipe(data);
			//delete data;
			return c;
		}
		else return NULL;
	}
	static void WriteRecipe(ComboRecipe recipe, int slotNr = 0)
	{
		unsigned char* data = recipe.ToRecipeData();

		ProcessMemoryManager::WriteMemory(SLOT1_OFFSET_POINTER, data, true, ComboRecipe::SLOT_DATA_SIZE * slotNr);

		//delete data;
	}
private:
	static const DWORD SLOT1_OFFSET_POINTER = 0x00BD3E94;
};

