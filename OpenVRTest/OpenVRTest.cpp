// OpenVRTest.cpp : Defines the entry point for the console application.
//

#include "VRWindow.h"

int main(int argc, char** argv)
{
	WindowManager wm(800, 400, "OpenVR Test");
	char* loadFilename = "models/dragon.obj";
	char* saveFilename = "saved/dragon.clr";
	switch (argc) {
	case 1:
		//Change to ricco
		loadFilename = "models/dragon.obj";
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
	}

	wm.paintingLoop(loadFilename, saveFilename);
}