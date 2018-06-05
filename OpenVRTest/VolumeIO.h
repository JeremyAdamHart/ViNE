#pragma once
#include <string>
#include <fstream>
#include "MeshInfoLoader.h"

bool saveVolume(std::string saveFileName, std::string objName, unsigned char* colors, int pointNum);
bool loadVolume(std::string saveFileName, renderlib::MeshInfoLoader* minfo, std::vector<unsigned char>* colors, std::string* objName);
