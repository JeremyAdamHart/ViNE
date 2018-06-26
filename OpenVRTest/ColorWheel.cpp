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

	generateColorWheelGeometry2();
}

vector<vec3> calculateNormals(vector<vec3>* points, vector<unsigned int>* indices) {
	vector<vec3> normals(points->size(), vec3(0));
	for (int i = 0; i < indices->size(); i += 3) {
		unsigned int ai = indices->at(i);
		unsigned int bi = indices->at(i + 1);
		unsigned int ci = indices->at(i + 2);
		vec3 a = points->at(ai);
		vec3 b = points->at(bi);
		vec3 c = points->at(ci);

		vec3 normal = cross(b - a, c - a);
		normals[ai] += normal;
		normals[bi] += normal;
		normals[ci] += normal;
	}

	for (int i = 0; i < normals.size(); i++) {
		normals[i] = normalize(normals[i]);
	}

	return normals;
}

void ColorWheel::thumbPos(vec2 pos) {
	const float PRESS_RAD = 0.2f;
	const float PRESS_DEPTH = 0.02f;

	auto geom = dynamic_cast<StreamGeometry<vec3, vec3, unsigned char>*>(getGeometryPtr());

	for (auto it : depressionList) {
		points[it.first] += modelspaceNormal()*it.second;
		geom->modify<POSITION>(it.first, points[it.first]);
	}
	depressionList.clear();

	for (int i = 0; i < points.size(); i++) {
		vec3 p = points[i];
		vec2 p_proj = vec2(
			dot(bx, p - origin) / dot(bx, bx),
			dot(by, p - origin) / dot(by, by));
		vec2 diff = p_proj - pos;
		float plane_dist = length(p_proj - pos);
		if (plane_dist < PRESS_RAD) {
			float z = sqrt(PRESS_RAD*PRESS_RAD - plane_dist*plane_dist)*PRESS_DEPTH;
			points[i] -= modelspaceNormal()*z;
			geom->modify<POSITION>(i, points[i]);
			depressionList[i] = z;
		}
	}

	geom->dump<POSITION>();

	normals = calculateNormals(&points, &indices);
	for (int i = 0; i < normals.size(); i++) {
		geom->modify<NORMAL>(i, normals[i]);
	}

	geom->dump<NORMAL>();

	geom->buffManager.endWrite();

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
		vec2 p_proj = vec2(
			dot(bx, points[i] - origin) / dot(bx, bx),
			dot(by, points[i] - origin) / dot(by, by));
		vec2 expand_proj = p_proj*1.f / 8.f;
		points[i] -= normal*RAISED_HEIGHT;
//		points[i] -= (points[i] - origin)/8.f;
		points[i] -= expand_proj.x*bx + expand_proj.y*by;
		geom->modify<POSITION>(i, points[i]);
	}

	//Increase new heights
	for (unsigned int i : affectedVertices[color]) {
		vec2 p_proj = vec2(
			dot(bx, points[i] - origin) / dot(bx, bx),
			dot(by, points[i] - origin) / dot(by, by));
		vec2 expand_proj = p_proj*1.f / 7.f;
		points[i] += expand_proj.x*bx + expand_proj.y*by;
//		points[i] += (points[i] - origin)/7.f;
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

vec3 circlePoint(vec3 origin, vec3 bx, vec3 by, float theta, float r) {
	return origin + bx*r*cos(theta) + by*r*sin(theta);
}

vec3 ColorWheel::modelspaceNormal() { return normalize(cross(bx, by)); }

void ColorWheel::generateSlice(float theta, float angularWidth, 
	int thetaDivisions, int radiusDivisions, unsigned char color) {
	int startIndex = points.size();
	float thetaStep = angularWidth / float(thetaDivisions - 1);
	float radiusStep = 1.f / float(radiusDivisions - 1);
	
	float t = theta;

	for (int i = 0; i < thetaDivisions; i++) {
		float r = 0;
		for (int j = 0; j < radiusDivisions; j++) {
			points.push_back(circlePoint(origin, bx, by, t, r));
			colors.push_back(color);
			affectedVertices[color].push_back(points.size() - 1);

			if (i != 0 && j != 0) {
				indices.push_back((i-1)*radiusDivisions + j - 1 + startIndex);
				indices.push_back((i-1)*radiusDivisions + j + startIndex);
				indices.push_back(i*radiusDivisions + j + startIndex);

				indices.push_back((i-1)*radiusDivisions + j - 1 + startIndex);
				indices.push_back(i*radiusDivisions + j + startIndex);
				indices.push_back(i*radiusDivisions + j - 1 + startIndex);

			}

			r += radiusStep;
		}

		t += thetaStep;
	}

	float r = 0.f;

	//Left side
	for (int i = 0; i < radiusDivisions; i++) {
		points.push_back(circlePoint(origin, bx, by, theta+angularWidth, r));
		colors.push_back(color);
		
		points.push_back(circlePoint(origin, bx, by, theta+angularWidth, r));
		colors.push_back(color);
		affectedVertices[color].push_back(points.size() - 1);

		if (i > 0) {
			indices.push_back(points.size() - 4);
			indices.push_back(points.size() - 3);
			indices.push_back(points.size() - 1);

			indices.push_back(points.size() - 4);
			indices.push_back(points.size() - 1);
			indices.push_back(points.size() - 2);
		}

		r += radiusStep;
	}

	//Right side
	r = 0.f;
	for (int i = 0; i < radiusDivisions; i++) {
		points.push_back(circlePoint(origin, bx, by, theta, r));
		colors.push_back(color);

		points.push_back(circlePoint(origin, bx, by, theta, r));
		colors.push_back(color);
		affectedVertices[color].push_back(points.size() - 1);

		if (i > 0) {
			indices.push_back(points.size() - 3);
			indices.push_back(points.size() - 4);
			indices.push_back(points.size() - 1);

			indices.push_back(points.size() - 1);
			indices.push_back(points.size() - 4);
			indices.push_back(points.size() - 2);
		}

		r += radiusStep;
	}

	//Back side
	t = theta;
	for (int i = 0; i < thetaDivisions; i++) {
		points.push_back(circlePoint(origin, bx, by, t, 1.f));
		colors.push_back(color);

		points.push_back(circlePoint(origin, bx, by, t, 1.f));
		colors.push_back(color);
		affectedVertices[color].push_back(points.size() - 1);

		if (i > 0) {
			indices.push_back(points.size() - 4);
			indices.push_back(points.size() - 2);
			indices.push_back(points.size() - 3);

			indices.push_back(points.size() - 2);
			indices.push_back(points.size() - 1);
			indices.push_back(points.size() - 3);
		}

		t += thetaStep;
	}

}

void ColorWheel::generateColorWheelGeometry2() {
	vec3 normal = modelspaceNormal();
	float thetaStep = 2.f*PI_F / float(colorNum);
	float theta = -thetaStep*0.5f;

	for (int i = 0; i < colorNum; i++) {
		
		generateSlice(theta, thetaStep, subdivisionNum, subdivisionNum*colorNum / 6, i);
		theta += thetaStep;
	}
	
	normals = calculateNormals(&points, &indices);

	auto geometry = new StreamGeometry<vec3, vec3, unsigned char>(points.size(), { true, true, false });
	geometry->loadElementArray(indices.size(), GL_STATIC_DRAW, indices.data());
	geometry->loadBuffer<POSITION>(points.data());
	geometry->loadBuffer<NORMAL>(normals.data());
	geometry->loadBuffer<COLOR>(colors.data());

	setGeometryContainer(geometry);
}

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
			points.push_back(circlePoint(origin, bx, by, thetaSub, 1.f));
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
		vec3 pointRight = circlePoint(origin, bx, by, theta-0.5f*thetaStep, 1.f);
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
		vec3 pointLeft = circlePoint(origin, bx, by, theta + 0.5f*thetaStep, 1.f);
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
			vec3 point = circlePoint(origin, bx, by, thetaSub, 1.f);
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