#pragma once
#include "ComboRecipe.h"
#include <vector>
static class ComboRecipeManager
{
public:
	static std::vector<ComboRecipe> ReadComboRecipes();
	static ComboRecipe ReadComboRecipe(int slotNr);
	static void WriteRecipe(ComboRecipe recipe, int slotNr = 0);
private:
	static const int SLOT1_OFFSET_POINTER = 0x00BD3E94;
};

