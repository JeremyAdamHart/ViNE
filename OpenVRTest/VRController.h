#pragma once

#include <openvr.h>
#include <Drawable.h>
#include <Texture.h>
#include <glm/glm.hpp>

class VRController: public renderlib::Drawable{
public:
	VRController(vr::TrackedDeviceIndex_t index, vr::IVRSystem *vrSystem, 
		vr::TrackedDevicePose_t pose, renderlib::TextureManager *texManager);

	vr::TrackedDeviceIndex_t index;

	enum {
		TRIGGER=0,
		AXIS_COUNT
	};
	enum {
		TRACKPAD=0,
		BUTTON_COUNT
	};

	glm::vec2 axes[AXIS_COUNT];
	bool buttons[BUTTON_COUNT];

	void updatePose(const vr::TrackedDevicePose_t &pose);
	void updateState(const vr::VRControllerState_t &state);
};