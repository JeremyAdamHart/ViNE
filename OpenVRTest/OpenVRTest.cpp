// OpenVRTest.cpp : Defines the entry point for the console application.
//

#include "VRWindow.h"
#include <string>

int main(int argc, char** argv)
{
	WindowManager wm(800, 400, "VR Segmenting");
	char* loadFilename = "untrackedmodels/riccoSurface_take3.obj";	//"models/dragon.obj";
	char* saveFilename = "saved/ricco.clr";
	int multisampling = 8;
	switch (argc) {
	case 1:
		//Change to ricco
		loadFilename = "untrackedmodels/riccoSurface_take3.obj";
		saveFilename = "saved/dragon.clr";
		break;
	case 2:
		loadFilename = argv[1];
		saveFilename = argv[1];
		break;
	case 3:
		loadFilename = argv[1];
		saveFilename = argv[2];
		break;
	case 4:
		loadFilename = argv[1];
		saveFilename = argv[2];
		multisampling = std::stoi(argv[3]);
	}

	wm.paintingLoop(loadFilename, saveFilename, multisampling);
}