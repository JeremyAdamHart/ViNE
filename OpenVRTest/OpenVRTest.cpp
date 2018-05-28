// OpenVRTest.cpp : Defines the entry point for the console application.
//

#include "VRWindow.h"

int main(int argc, char** argv)
{
	WindowManager wm(800, 400, "OpenVR Test");
	wm.paintingLoop();
}