#include "asset_builder_tool.h"

#include <stdio.h>
#include <stdlib.h>
#include <iostream>

struct asset_system {

};

void WriteAssetFile(asset_system* Assets, char* FileName) {
	FILE* fp = fopen(FileName, "wb");

	if (fp) {

		

		fclose(fp);
	}
}

int main() {



	system("pause");
	return(0);
}