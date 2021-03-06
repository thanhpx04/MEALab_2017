#include <vector>
#include <stdio.h>
#include <string.h>
#include <fstream>
#include <iostream>
#include <stdlib.h>

using namespace std;

#include "../imageModel/Matrix.h"
#include "../imageModel/Point.h"

#include "TPSReader.h"
#include "JPEGReader.h"

#include "Reader.h"


//====================================================== JPEG File ==================================================

ptr_RGBMatrix readJPGToRGB(const char* filename) {
	return decompressJPEG(filename);
}
void saveRGB(const char* filename, ptr_RGBMatrix rgbMatrix){
	RGB2JPEG(filename,rgbMatrix);
}
void saveGrayScale(const char* filename, ptr_IntMatrix grayMatrix){
	Gray2JPEG(filename,grayMatrix);
}
// ============================================================== TPS File =====================================================
vector<ptr_Point> readTPSFile(const char* filename) {
	return readTPS(filename);
}

