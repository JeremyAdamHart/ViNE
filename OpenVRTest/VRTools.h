#pragma once

#include <openvr.h>
#include <glm/glm.hpp>
#include <ElementGeometry.h>
#include <Drawable.h>
#include <Texture.h>

vec3 toVec3(vr::HmdVector3_t vec);
vec2 toVec2(vr::HmdVector2_t vec);
vec3 getTranslation(vr::HmdMatrix34_t matrix);
mat3 getRotation(vr::HmdMatrix34_t matrix);
mat4 toMat4(const vr::HmdMatrix44_t &hmdMat);
mat4 toMat4(const vr::HmdMatrix34_t &hmdMat);

void openvrRenderModelToDrawable(Drawable *drawable,
	vr::RenderModel_t *openvrModel, TextureManager *texManager);

