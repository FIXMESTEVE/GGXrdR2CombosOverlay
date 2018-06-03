#include "ComboRecipeManager.h"
#include "ProcessMemoryManager.h"
#include <vector>

std::vector<ComboRecipe> ComboRecipeManager::ReadComboRecipes()
{
	unsigned char* comboRecipeData = ProcessMemoryManager::ReadMemory(SLOT1_OFFSET_POINTER, ComboRecipe::SLOT_DATA_SIZE * 5, true);

	ComboRecipe combo1 = ComboRecipe(comboRecipeData, 0);
	ComboRecipe combo2 = ComboRecipe(comboRecipeData, 1);
	ComboRecipe combo3 = ComboRecipe(comboRecipeData, 2);
	ComboRecipe combo4 = ComboRecipe(comboRecipeData, 3);
	ComboRecipe combo5 = ComboRecipe(comboRecipeData, 4);

	std::vector<ComboRecipe> arr = { combo1, combo2, combo3, combo4, combo5 };

	return arr;
}

ComboRecipe ComboRecipeManager::ReadComboRecipe(int slotNr)
{
	unsigned char* data = ProcessMemoryManager::ReadMemory(SLOT1_OFFSET_POINTER, ComboRecipe::SLOT_DATA_SIZE, true, slotNr * ComboRecipe::SLOT_DATA_SIZE);

	return data != NULL ? ComboRecipe(data) : NULL;
}

void ComboRecipeManager::WriteRecipe(ComboRecipe recipe, int slotNr)
{
	unsigned char* data = recipe.ToRecipeData();

	ProcessMemoryManager::WriteMemory(SLOT1_OFFSET_POINTER, data, true, ComboRecipe::SLOT_DATA_SIZE * slotNr);
}
