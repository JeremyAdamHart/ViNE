// OpenVRTest.cpp : Defines the entry point for the console application.
//

#include "VRWindow.h"
#include <string>
#include <iostream>
#include <fstream>

#include "ConvexHull.h"


int main(int argc, char** argv)
{
	using namespace std;
	vector<unsigned char> colors = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11 };

	ofstream f("test.txt", ios::binary);
	if (!f.is_open()) {
		printf("Could not open\n");
		return 1;
	}

	f << "Start" << endl;

	for (int i = 0; i < colors.size(); i++) {
		f << char(colors[i]);
	}

	f.close();



	SlotMap<int> map;

	SlotMap<int>::Index a = map.add(1);
	SlotMap<int>::Index b = map.add(2);
	SlotMap<int>::Index c = map.add(3);
	map.remove(b);
	SlotMap<int>::Index d = map.add(4);
	SlotMap<int>::Index e = map.add(5);

	for (auto it = map.begin(); it != map.end(); ++it) {
		std::cout << (*it) << std::endl;
	}
	/*
	HalfEdgeMesh<glm::vec3> mesh;
	generateTetrahedron(mesh, glm::vec3(0, 0, 0), glm::vec3(1, 0, 0), glm::vec3(0, 1, 0), glm::vec3(0, 0, 1));

	std::vector<glm::vec3> points;
	std::vector<int> indices;
	halfEdgeToFaceList(&points, &indices, mesh);
	*/

	WindowManager wm(926, 1028, "VR Segmenting");
	char* loadFilename = "icosahedron.ply";	//"untrackedmodels/riccoSurface_take3.obj";	//"models/dragon.obj";
	char* saveFilename = "saved/default.clr";
	int multisampling = 4;
	switch (argc) {
	case 1:
		//Change to ricco
		//loadFilename = "saved/stitchedslicesBlueTraced.clr";
		//loadFilename = "saved/GerberaBackwardsVeins.clr";
		//loadFilename = "saved/IsolatedVeinColored.clr";
		//loadFilename = "untrackedmodels/Helianthus.ply";
		//loadFilename = "untrackedmodels/dragon.ply";
		//loadFilename = "saved/stitchedslices1_3000x3000x982_gaussian-1.5_Iso-21850.clr";
		//loadFilename = "untrackedmodels/Craspedia2.ply";	// "untrackedmodels/Helianthus4.ply";	//"untrackedmodels/GRCD2RNA.ply";	//"models/Cube.obj";	//"models/icosahedron.ply";
		loadFilename = "saved/Helianthus2Vein.clr";
		//loadFilename = "saved/Helianthus2.clr";
		//loadFilename = "saved/default.clr";
		//loadFilename = "saved/lineendings.clr";
		//loadFilename = "saved/GerberaNoHair.clr";
		//loadFilename = "saved/Craspedia.clr";
		//loadFilename = "saved/Florets3Sectors.clr";
		//loadFilename = "saved/FloretsMarked.clr";
		//loadFilename = "saved/default.clr";	//"untrackedmodels/Sample3_q00iso115.ply";
		saveFilename = "saved/default.clr";
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

	wm.paintingLoopIndexedMT(loadFilename, saveFilename, multisampling);
}