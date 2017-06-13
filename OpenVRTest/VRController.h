#pragma once

#include <openvr.h>
#include <Drawable.h>
#include <Texture.h>

class VRController: public Drawable{
	vr::IVRSystem *vrSystem;
public:
	VRController(vr::TrackedDeviceIndex_t index, vr::IVRSystem *vrSystem, 
		vr::TrackedDevicePose_t pose, TextureManager *texManager);

	vr::TrackedDeviceIndex_t index;

	void updatePose(const vr::TrackedDevicePose_t &pose);
};