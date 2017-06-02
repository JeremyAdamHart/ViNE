#pragma once

#include <openvr.h>
#include <glm/glm.hpp>
#include <ElementGeometry.h>
#include <Drawable.h>
#include <TextureMat.h>

vec3 toVec3(vr::HmdVector3_t vec);
vec2 toVec2(vr::HmdVector2_t vec);

Drawable *openvrRenderModelToDrawable(const vr::RenderModel_t &openvrModel);