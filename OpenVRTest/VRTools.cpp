#include "VRTools.h"
#include <TextureCreation.h>

Drawable *openvrRenderModelToDrawable(const vr::RenderModel_t &openvrModel)
{

	//Extract data into individual arrays
	vec3 *positions = new vec3[openvrModel.unVertexCount];
	vec3 *normals = new vec3[openvrModel.unVertexCount];
	vec2 *texCoords = new vec2[openvrModel.unVertexCount];

	for (int i = 0; i < openvrModel.unVertexCount; i++) {
		positions[i] = toVec3(openvrModel.rVertexData[i].vPosition);
		normals[i] = toVec3(openvrModel.rVertexData[i].vNormal);
		texCoords[i] = { openvrModel.rVertexData[i].rfTextureCoord[0],
						openvrModel.rVertexData[i].rfTextureCoord[1] };
	}

	unsigned int *indices = new unsigned int[openvrModel.unTriangleCount * 3];
	for (int i = 0; i < openvrModel.unTriangleCount * 3; i++) {
		indices[i] = openvrModel.rIndexData[i];
	}

	ElementGeometry *geom = new ElementGeometry(positions,
		normals, texCoords, indices, openvrModel.unVertexCount, 
		openvrModel.unTriangleCount * 3);

	vr::RenderModel_TextureMap_t openvrTex;
	vr::VRRenderModels()->LoadTexture_Async(openvrModel.diffuseTextureId, &openvrTex);

	/*Texture tex = createTexture2D(
		TexInfo(GL_TEXTURE_2D, )
	)*/
}

vec3 toVec3(vr::HmdVector3_t vec) {
	return vec3(vec.v[0], vec.v[1], vec.v[2]);
}

vec2 toVec2(vr::HmdVector2_t vec) {
	return vec2(vec.v[0], vec.v[2]);
}