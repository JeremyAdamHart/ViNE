#include "VRController.h"
#include <glmSupport.h>
#include "VRTools.h"

VRController::VRController(vr::TrackedDeviceIndex_t index, vr::IVRSystem *vrSystem, 
	vr::TrackedDevicePose_t pose, TextureManager *texManager) : 
	vrSystem(vrSystem), index(index)
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
		switch (error) {
		case vr::VRRenderModelError_BufferTooSmall:
			printf("RenderModelError - BufferTooSmall\n");
			break;
		case vr::VRRenderModelError_InvalidArg:
			printf("RenderModelError - InvalidArg\n");
			break;
		case vr::VRRenderModelError_InvalidModel:
			printf("RenderModelError - InvalidModel\n");
			break;
		case vr::VRRenderModelError_InvalidTexture:
			printf("RenderModelError - InvalidTexture\n");
			break;
		case vr::VRRenderModelError_Loading:
			printf("RenderModelError - Loading\n");
			break;
		case vr::VRRenderModelError_MultipleShapes:
			printf("RenderModelError - MultipleShapes\n");
			break;
		case vr::VRRenderModelError_MultipleTextures:
			printf("RenderModelError - MultipleTextures\n");
			break;
		case vr::VRRenderModelError_NoShapes:
			printf("RenderModelError - NoShapes\n");
			break;
		case vr::VRRenderModelError_NotEnoughNormals:
			printf("RenderModelError - NotEnoughNormals\n");
			break;
		case vr::VRRenderModelError_NotEnoughTexCoords:
			printf("RenderModelError - NotEnoughTexCoords\n");
			break;
		case vr::VRRenderModelError_NotSupported:
			printf("RenderModelError - NotSupported\n");
			break;
		case vr::VRRenderModelError_TooManyVertices:
			printf("RenderModelError - TooManyVertices\n");
			break;
		}
	}
}

void VRController::updatePose(const vr::TrackedDevicePose_t &pose) {
	vr::HmdMatrix34_t poseMatrix = pose.mDeviceToAbsoluteTracking;
	position = getTranslation(poseMatrix);
	orientation = quat_cast(getRotation(poseMatrix));
	orientation = normalize(orientation);
}