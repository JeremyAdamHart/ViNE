// OpenVRTest.cpp : Defines the entry point for the console application.
//

#include "VRWindow.h"

int main()
{
	WindowManager wm(1600, 800, "OpenVR Test");
	wm.paintingLoop();
}