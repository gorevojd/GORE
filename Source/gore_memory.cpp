#include "gore_memory.h"
#include <iostream>

void ClearStackedMemory(stacked_memory* Mem) {
	memset(Mem->BaseAddress, 0, Mem->MaxSize);
}