#pragma once
#include <Drawable.h>
#include <vector>

//Typically needs ColorSetMat
class ColorWheel : public renderlib::Drawable {
	std::vector<glm::vec3> points;
	std::vector<glm::vec3> normals;
	std::vector<unsigned char> colors;
	std::vector<glm::vec2> uvCoords;
	std::vector<unsigned int> indices;
	glm::vec3 origin;
	glm::vec3 bx;
	glm::vec3 by;
	int colorNum;
	int subdivisionNum;
	int selectedColor;
	std::map<unsigned int, float> depressionList;

	const float RAISED_HEIGHT = 0.01f;

	std::vector<std::vector<unsigned int>> affectedVertices;

	enum {
		POSITION=0,
		NORMAL,
		COLOR
	};

	void generateSlice(float theta, float angularWidth, int thetaDivisions, 
		int radiusDivisions, unsigned char color);
	void generateColorWheelGeometry();
	void generateColorWheelGeometry2();
	glm::vec3 modelspaceNormal();

public:
	ColorWheel(glm::vec3 origin, glm::vec3 bx, glm::vec3 by, 
		int colorNum, int subdivisionNum);

	void thumbPos(glm::vec2 pos);

	void pressColor();
	void unpressColor();
	void selectColor(int color);
	glm::vec3 trackpadLightPosition(float dist);
};