#include "VRController.h"
#include <glmSupport.h>
#include "VRTools.h"

using namespace renderlib;

string getRenderModelErrorString(vr::EVRRenderModelError error) {
	switch (error) {
	case vr::VRRenderModelError_BufferTooSmall:
		return "BufferTooSmall";
	case vr::VRRenderModelError_InvalidArg:
		return "InvalidArg";
	case vr::VRRenderModelError_InvalidModel:
		return "InvalidModel";
	case vr::VRRenderModelError_InvalidTexture:
		return "InvalidTexture";
	case vr::VRRenderModelError_Loading:
		return "Loading";
	case vr::VRRenderModelError_MultipleShapes:
		return "MultipleShapes";
	case vr::VRRenderModelError_MultipleTextures:
		return "MultipleTextures";
	case vr::VRRenderModelError_NoShapes:
		return "NoShapes";
	case vr::VRRenderModelError_NotEnoughNormals:
		return "NotEnoughNormals";
	case vr::VRRenderModelError_NotEnoughTexCoords:
		return "NotEnoughTexCoords";
	case vr::VRRenderModelError_NotSupported:
		return "NotSupported";
	case vr::VRRenderModelError_TooManyVertices:
		return "TooManyVertices";
	}
}

VRController::VRController(vr::TrackedDeviceIndex_t index, vr::IVRSystem *vrSystem, 
	vr::TrackedDevicePose_t pose, TextureManager *texManager) : 
	index(index)
{
	updatePose(pose);

	char nameBuffer[1024];
	vrSystem->GetStringTrackedDeviceProperty(index, 
		vr::Prop_RenderModelName_String, nameBuffer, 1024);

	vr::RenderModel_t *renderModel;
	vr::EVRRenderModelError error = vr::VRRenderModels()->LoadRenderModel_Async(
		nameBuffer, &renderModel);

	while(error == vr::VRRenderModelError_Loading)
		error = vr::VRRenderModels()->LoadRenderModel_Async(
			nameBuffer, &renderModel);

	if(error == vr::VRRenderModelError_None)
		openvrRenderModelToDrawable(this, renderModel, texManager);
	else {
		printf("RenderModelError - %s\n", getRenderModelErrorString(error).c_str());
	}
}

void VRController::updatePose(const vr::TrackedDevicePose_t &pose) {
	vr::HmdMatrix34_t poseMatrix = pose.mDeviceToAbsoluteTracking;
	position = getTranslation(poseMatrix);
	orientation = normalize(quat_cast(getRotation(poseMatrix)));
}

void VRController::updateState(const vr::VRControllerState_t &state) {
	axes[TRIGGER] = vec2(state.rAxis[1].x, 0.f);
	buttons[TRACKPAD] = vr::ButtonMaskFromId(vr::k_EButton_SteamVR_Touchpad) && state.ulButtonPressed;
}