// OpenVRTest.cpp : Defines the entry point for the console application.
//

#include "VRWindow.h"

int main(int argc, char** argv)
{
	WindowManager wm(800, 400, "VR Segmenting");
	char* loadFilename = "untrackedmodels/riccoSurface_take3.obj";	//"models/dragon.obj";
	char* saveFilename = "saved/ricco.clr";
	switch (argc) {
	case 1:
		//Change to ricco
		loadFilename = "untrackedmodels/riccoSurface_take3.obj";
		saveFilename = "saved/ricco.clr";
		break;
	case 2:
		loadFilename = argv[1];
		saveFilename = argv[1];
		break;
	case 3:
		loadFilename = argv[1];
		saveFilename = argv[2];
		break;
	}

	wm.paintingLoop(loadFilename, saveFilename, 8);
}