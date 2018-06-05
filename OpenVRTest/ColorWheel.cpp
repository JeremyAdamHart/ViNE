#include "ColorWheel.h"
#include <StreamGeometry.h>

const float PI_F = 3.1415927;

using namespace std;
using namespace glm;
using namespace renderlib;

ColorWheel::ColorWheel(glm::vec3 origin, glm::vec3 bx, glm::vec3 by, int colorNum, int subdivisionNum)
	: origin(origin), bx(bx), by(by), colorNum(colorNum), subdivisionNum(subdivisionNum),
	selectedColor(colorNum), affectedVertices(colorNum+1)
{

	generateColorWheelGeometry();
}

void ColorWheel::selectColor(int color) {
	if (selectedColor == color) return;

	auto geom = dynamic_cast<StreamGeometry<vec3, vec3, unsigned char>*>(getGeometryPtr());

	if (geom == nullptr) {
		printf("StreamGeometry::selectColor() -- Incorrect geometry container\n");
		return;
	}

	vec3 normal = normalize(cross(bx, by));

	//Reset old heights
	for (unsigned int i : affectedVertices[selectedColor]) {
		points[i] -= normal*RAISED_HEIGHT;
		points[i] -= (points[i] - origin)/8.f;
		geom->modify<POSITION>(i, points[i]);
	}

	//Increase new heights
	for (unsigned int i : affectedVertices[color]) {
		points[i] += (points[i] - origin)/7.f;
		points[i] += normal*RAISED_HEIGHT;
		geom->modify<POSITION>(i, points[i]);
	}
	geom->dump<POSITION>();
	geom->buffManager.endWrite();

	selectedColor = color;
}

vec3 ColorWheel::trackpadLightPosition(float dist) {
	vec4 lightPosModelspace = vec4(origin + modelspaceNormal()*dist, 1.f);
	vec3 lightPosWorldspace = getTransform()*lightPosModelspace;
	return vec3(lightPosWorldspace);
}

vec3 circlePoint(vec3 origin, vec3 bx, vec3 by, float theta) {
	return origin + bx*cos(theta) + by*sin(theta);
}

vec3 ColorWheel::modelspaceNormal() { return normalize(cross(bx, by)); }

void ColorWheel::generateColorWheelGeometry() {
	vec3 normal = modelspaceNormal();
	float theta = 0.f;
	float thetaStep = 2.f*PI_F / float(colorNum);
	float thetaSubStep = thetaStep / float(subdivisionNum - 1);
	for (int i = 0; i < colorNum; i++) {
		points.push_back(origin);
		normals.push_back(normal);
		colors.push_back(i);
	}
	for (int i = 0; i < colorNum; i++) {
		float thetaSub = theta - thetaStep*0.5f;
		//Vertical triangle points
		for (int j = 0; j < subdivisionNum; j++) {
			points.push_back(circlePoint(origin, bx, by, thetaSub));
			normals.push_back(normal);
			colors.push_back(i);
			affectedVertices[i].push_back(points.size() - 1);
			if (j != 0) {
				indices.push_back(i);
				indices.push_back(points.size() - 2);
				indices.push_back(points.size() - 1);
			}

			thetaSub += thetaSubStep;
		}
		theta += thetaStep;
	}

	//Vertical triangle points
	theta = 0.f;
	for (int i = 0; i < colorNum; i++) {
		//Right side
		vec3 pointRight = circlePoint(origin, bx, by, theta-0.5f*thetaStep);
		vec3 normalRight = normalize(cross(pointRight - origin, normal));

		points.push_back(pointRight);
		normals.push_back(normalRight);
		colors.push_back(i);
		affectedVertices[i].push_back(points.size() - 1);

		points.push_back(origin);
		normals.push_back(normalRight);
		colors.push_back(i);
		
		points.push_back(pointRight);
		normals.push_back(normalRight);
		colors.push_back(i);

		indices.push_back(points.size() - 3);
		indices.push_back(points.size() - 2);
		indices.push_back(points.size() - 1);

		//Left side
		vec3 pointLeft = circlePoint(origin, bx, by, theta + 0.5f*thetaStep);
		vec3 normalLeft = normalize(cross(pointLeft - origin, normal));

		points.push_back(pointLeft);
		normals.push_back(normalLeft);
		colors.push_back(i);

		points.push_back(origin);
		normals.push_back(normalLeft);
		colors.push_back(i);

		points.push_back(pointLeft);
		normals.push_back(normalLeft);
		colors.push_back(i);
		affectedVertices[i].push_back(points.size() - 1);

		indices.push_back(points.size() - 3);
		indices.push_back(points.size() - 2);
		indices.push_back(points.size() - 1);

		theta += thetaStep;
	}

	//Outside triangle points
	theta = 0.f;
	for (int i = 0; i < colorNum; i++) {
		float thetaSub = theta - thetaStep*0.5f;
		//Vertical triangle points
		for (int j = 0; j < subdivisionNum; j++) {
			vec3 point = circlePoint(origin, bx, by, thetaSub);
			vec3 normal = normalize(point - origin);
			points.push_back(point);
			normals.push_back(normal);
			colors.push_back(i);

			points.push_back(point);
			normals.push_back(normal);
			colors.push_back(i);
			affectedVertices[i].push_back(points.size() - 1);

			if (j != 0) {
				indices.push_back(points.size() - 4);
				indices.push_back(points.size() - 2);
				indices.push_back(points.size() - 1);

				indices.push_back(points.size() - 4);
				indices.push_back(points.size() - 1);
				indices.push_back(points.size() - 3);
			}

			thetaSub += thetaSubStep;
		}
		theta += thetaStep;
	}


	auto geometry = new StreamGeometry<vec3, vec3, unsigned char>(points.size(), { true, false, false });
	geometry->loadElementArray(indices.size(), GL_STATIC_DRAW, indices.data());
	geometry->loadBuffer<POSITION>(points.data());
	geometry->loadBuffer<NORMAL>(normals.data());
	geometry->loadBuffer<COLOR>(colors.data());

	setGeometryContainer(geometry);
}