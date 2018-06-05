#include "VolumeIO.h"
#include <stdio.h>
#include <vector>

using namespace std;
using namespace renderlib;

bool saveVolume(std::string saveFileName, std::string objName, unsigned char* colors, int pointNum) {
	std::ofstream f(saveFileName.c_str());
	if (!f.is_open()) {
		printf("VolumeIO::saveVolume - File could not be opened\n");
		return false;
	}

	f << objName.c_str() << endl;

	for (int i = 0; i < pointNum; i++) {
		f << colors[i];
	}

	printf("pointNum = %d\n", pointNum);
	return true;
}

bool loadVolume(std::string saveFileName, MeshInfoLoader* minfo, std::vector<unsigned char>* colors, std::string* objName) {
	std::ifstream f(saveFileName.c_str());
	if (!f.is_open()) {
		printf("VolumeIO::loadVolume - File could not be opened\n");
		return false;
	}

	const int BUFFER_SIZE = 1024;
	char buffer[BUFFER_SIZE];
	f.getline(buffer, BUFFER_SIZE);
	unsigned char value;
	while (f >> value) {
		colors->push_back(value);
	}

	if (!minfo->loadModel(buffer)) {
		return false;
	}

	if (minfo->vertices.size() != colors->size()) {
		printf("VolumeIO::loadVolume - Vertex and color size not equal\n");
		printf("\tVertice size = %d Color size = %d\n", minfo->vertices.size(), colors->size());
		return false;
	}

	(*objName) = buffer;

	return true;
}