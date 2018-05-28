#include "BubbleShader.h"
#include "ColorMat.h"

using namespace glm;

namespace renderlib {

enum {
	VP_MATRIX_LOCATION = ColorMat::COUNT,
	M_MATRIX_LOCATION,
	VIEW_LOCATION,
	COUNT
};

vector<pair<GLenum, string>> BubbleShader::defaultShaders() {
	return{ { GL_VERTEX_SHADER, "shaders/bubbleShader.vert" },
	{ GL_FRAGMENT_SHADER, "shaders/bubbleShader.frag" } };
}

BubbleShader::BubbleShader(map<GLenum, string> defines):SimpleShader(BubbleShader::defaultShaders(), defines) {
	calculateUniformLocations();
}

//Alt shader should use the same uniforms
BubbleShader::BubbleShader(vector<pair<GLenum, string>> alt_shaders, map<GLenum, string> defines) 
	:SimpleShader(alt_shaders, defines) {
	calculateUniformLocations();
}

void BubbleShader::calculateUniformLocations() {
	uniformLocations.clear();		//Clear old locations
	glUseProgram(programID);

	//Material uniforms
	uniformLocations.resize(COUNT);
	uniformLocations[ColorMat::COLOR_LOCATION] = glGetUniformLocation(programID, "color");

	//Other uniforms
	uniformLocations[VP_MATRIX_LOCATION] = glGetUniformLocation(programID,
		"view_projection_matrix");
	uniformLocations[M_MATRIX_LOCATION] = glGetUniformLocation(programID,
		"model_matrix");
	uniformLocations[VIEW_LOCATION] = glGetUniformLocation(programID,
		"view_position");
}

void BubbleShader::loadUniforms(const mat4& vp_matrix, const mat4& m_matrix, vec3 viewLocation) {
	glUniformMatrix4fv(uniformLocations[VP_MATRIX_LOCATION], 1, false, &vp_matrix[0][0]);
	glUniformMatrix4fv(uniformLocations[M_MATRIX_LOCATION], 1, false, &m_matrix[0][0]);
	glUniform3f(uniformLocations[VIEW_LOCATION], viewLocation.x, viewLocation.y, viewLocation.z);
}

void BubbleShader::draw(const Camera &cam, Drawable &obj) {
	glUseProgram(programID);
	loadUniforms(cam.getProjectionMatrix()*cam.getCameraMatrix(), obj.getTransform(), cam.getPosition());
	obj.loadUniforms(ColorMat::id, &uniformLocations[0]);

	obj.getGeometry().drawGeometry();
	glUseProgram(0);
}

}
