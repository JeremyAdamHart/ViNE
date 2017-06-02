// OpenVRTest.cpp : Defines the entry point for the console application.
//

#include "VRWindow.h"

int main()
{
	WindowManager wm(800, 400, "OpenVR Test");
	wm.mainLoop();
}