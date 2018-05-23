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

VRController::VRController() :renderModel(nullptr) {

}

VRController::VRController(vr::TrackedDeviceIndex_t index, vr::IVRSystem *vrSystem,
	vr::TrackedDevicePose_t pose, TextureManager *texManager) :
	index(index)
{
	updatePose(pose);

	char nameBuffer[1024];
	vrSystem->GetStringTrackedDeviceProperty(index,
		vr::Prop_RenderModelName_String, nameBuffer, 1024);

	vr::EVRRenderModelError error = vr::VRRenderModels()->LoadRenderModel_Async(
		nameBuffer, &renderModel);

	while (error == vr::VRRenderModelError_Loading)
		error = vr::VRRenderModels()->LoadRenderModel_Async(
			nameBuffer, &renderModel);

	if (error == vr::VRRenderModelError_None)
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
	//Mild hack to get index of trigger. Better approach would be to use https://github.com/ValveSoftware/openvr/issues/56
	int triggerIndex = vr::k_EButton_SteamVR_Trigger - vr::k_EButton_Axis0;
	int trackpadIndex = vr::k_EButton_SteamVR_Touchpad - vr::k_EButton_Axis0;
	axes[TRIGGER_AXIS] = vec2(state.rAxis[triggerIndex].x, 0.f);
	axes[TRACKPAD_AXIS] = vec2(state.rAxis[trackpadIndex].x, state.rAxis[trackpadIndex].y);
	buttons[TRACKPAD_BUTTON] = vr::ButtonMaskFromId(vr::k_EButton_SteamVR_Touchpad) & state.ulButtonPressed;
	buttons[GRIP_BUTTON] = vr::ButtonMaskFromId(vr::k_EButton_Grip) & state.ulButtonPressed;
	buttons[TRIGGER_BUTTON] = vr::ButtonMaskFromId(vr::k_EButton_SteamVR_Trigger) & state.ulButtonPressed;
	buttons[TRACKPAD_BUTTON] = vr::ButtonMaskFromId(vr::k_EButton_SteamVR_Touchpad) & state.ulButtonPressed;
	buttons[MENU_BUTTON] = vr::ButtonMaskFromId(vr::k_EButton_ApplicationMenu) & state.ulButtonPressed;
	trackpadTouched = vr::ButtonMaskFromId(vr::k_EButton_SteamVR_Touchpad) & state.ulButtonTouched;
}

void VRController::loadModelMatrixOldOpenGL() const {
	mat4 transform = getTransform();

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glMultMatrixf(&transform[0][0]);

}

VRSceneTransform::VRSceneTransform() :
	Object(vec3(0.f), quat()), scale(1.f),
	velocity(0.f), angularVelocity(), controllers(nullptr), 
	rotationMode(ONE_HAND_PLUS_SCALE), rotationOrigin(ORIGIN_MODEL) {}


VRSceneTransform::VRSceneTransform(vector<VRController> *controllers) :
	Object(vec3(0.f), quat()), scale(1.f),
	velocity(0.f), angularVelocity(), controllers(controllers), rotationMode(ONE_HAND_PLUS_SCALE), rotationOrigin(ORIGIN_MODEL) {
	linkControllers(controllers);
}

void VRSceneTransform::linkControllers(vector<VRController> *newControllers) {
	controllers = newControllers;
	lastPosition.resize(controllers->size());
	lastOrientation.resize(controllers->size());

	for (int i = 0; i < controllers->size(); i++) {
		lastPosition[i] = controllers->at(i).getPos();
		lastOrientation[i] = controllers->at(i).getOrientationQuat();
	}
}

void VRSceneTransform::setPosition(glm::vec3 newPos) { position = newPos; }

mat4 VRSceneTransform::getTransform() const {
	mat4 rigidTransform = Object::getTransform();
	mat4 scaleMatrix = mat4(
		scale, 0, 0, 0,
		0, scale, 0, 0,
		0, 0, scale, 0,
		0, 0, 0, 1);

	return rigidTransform*scaleMatrix;
}

void VRSceneTransform::multMatrixOldOpenGL() {
	/*	mat4 rigidTransform = getTransform();
	mat4 scaleMatrix = mat4(
	scale, 0, 0, 0,
	0, scale, 0, 0,
	0, 0, scale, 0,
	0, 0, 0, 1);
	*/
	mat4 transform = getTransform();	//rigidTransform*scaleMatrix;

	glMatrixMode(GL_MODELVIEW_MATRIX);
	glMultMatrixf(&transform[0][0]);
}

vec3 makePerpendicular(vec3 vector, vec3 perpendicularTo) {
	vec3 proj = dot(vector, perpendicularTo) / dot(perpendicularTo, perpendicularTo)*perpendicularTo;
	vec3 perpendicular = vector - proj;
	float magnitude = length(perpendicular);
	if (magnitude > 0.000001)
		return perpendicular / magnitude;
	else
		return vec3(0);
}

vec3 toVec3(quat q) { return vec3(q.x, q.y, q.z); }

quat projectedQuaternionDiff(quat a, quat b, vec3 projAxis) {
	quat diffQuat = normalize(b)*inverse(normalize(a));
	float sinTheta = dot(normalize(projAxis), toVec3(diffQuat));
	float w = sqrt(1 - sinTheta*sinTheta);
	return quat(w, float(sinTheta)*normalize(projAxis));
}

quat quaternionDiff(quat a, quat b) {
	return normalize(normalize(b)*inverse(normalize(a)));
}

//Not currently incorporating time - FIX
void VRSceneTransform::updateTransform(float deltaTime) {

	static vec3 rotationCenter = vec3(0.f, 0.f, 0.f);
	float scaleChange = 1.f;

	//Get indices of controllers which have the grip pressed
	std::vector<int> gripsPressed;
	for (int i = 0; i < controllers->size(); i++) {
		if (controllers->at(i).buttons[VRController::GRIP_BUTTON])
			gripsPressed.push_back(i);
	}

	switch (gripsPressed.size()) {
	case 1:
	{
		int index = gripsPressed[0];
		velocity = controllers->at(index).getPos() - lastPosition[index];
		angularVelocity = slerp(angularVelocity, quat(), 0.1f);
	//	angularVelocity = quat();
		break;
	}
	case 2:
	{
		int indexA = gripsPressed[0];
		int indexB = gripsPressed[1];

		velocity = vec3(0.f);
		angularVelocity = quat();

		vec3 axisA = lastPosition[indexA] - lastPosition[indexB];
		vec3 axisB = controllers->at(indexA).getPos()
			- controllers->at(indexB).getPos();
		float lengthA = length(axisA);
		float lengthB = length(axisB);

		if (rotationMode == HANDLEBAR) {
			axisA = axisA / lengthA;
			axisB = axisB / lengthB;


			rotationCenter = 0.5f*(controllers->at(0).getPos() + controllers->at(1).getPos());


			vec3 rotAxis = cross(axisA, axisB);
			if (length(rotAxis) > 0.0001f) {
				float angle = asin(length(rotAxis));
				angularVelocity = angleAxis(angle, normalize(rotAxis));
			}
		}
		else if(rotationMode == ONE_HAND_PLUS_SCALE){
			quat lastOrntnA = lastOrientation[indexA];
			quat lastOrntnB = lastOrientation[indexB];
			quat currOrntnA = controllers->at(indexA).getOrientationQuat();
			quat currOrntnB = controllers->at(indexB).getOrientationQuat();

			quat orntnDiffA = projectedQuaternionDiff(lastOrntnA, currOrntnA, axisB);
			quat orntnDiffB = projectedQuaternionDiff(lastOrntnB, currOrntnB, axisB);
			orntnDiffB = quaternionDiff(lastOrntnB, currOrntnB);
			angularVelocity = orntnDiffB;
			rotationCenter = controllers->at(indexB).getPos();

		}
		
		scaleChange = lengthB / lengthA;
		scale *= scaleChange;		//Rescale model

		break;
	}
	default:
	{
		velocity *= 0.99f;
		angularVelocity = slerp(angularVelocity, quat(), 0.1f);
	}
	}

	//Integrate velocities
	//Scene centered rotation
	if (rotationOrigin == ORIGIN_CONTROLLER) {
		position = vec3(
			toMat4(normalize(angularVelocity))*vec4(scaleChange*(position - rotationCenter), 1))
			+ rotationCenter;
	}
	position += velocity;
	orientation = normalize(angularVelocity*orientation);

	//Save positions
	for (int i = 0; i < lastPosition.size(); i++) {
		lastPosition[i] = controllers->at(i).getPos();
		lastOrientation[i] = controllers->at(i).getOrientationQuat();
	}
}

bool VRSceneTransform::multMatrixPreviewTransform(float modelScale) {
	std::vector<int> gripsPressed;
	for (int i = 0; i < controllers->size(); i++) {
		if (controllers->at(i).buttons[VRController::GRIP_BUTTON])
			gripsPressed.push_back(i);
	}
	if (gripsPressed.size() < 2)
		return false;

	vec3 controllerPos[2];
	controllerPos[0] = controllers->at(gripsPressed[0]).getPos();
	controllerPos[1] = controllers->at(gripsPressed[1]).getPos();
	float controllerDist = length(controllerPos[0] - controllerPos[1]);
	float newScale = controllerDist / (2.f*modelScale);
	vec3 controllerMidpoint = (controllerPos[0] + controllerPos[1])*0.5f;

	mat4 translationToMidpoint = translateMatrix(controllerMidpoint);
	mat4 scaleBetweenControllers = scaleMatrix(newScale);

	mat4 modelMatrix = translationToMidpoint*scaleBetweenControllers*getOrientationMat4();

	glMatrixMode(GL_MODELVIEW);
	//	glPushMatrix();
	glMultMatrixf(&modelMatrix[0][0]);

	return true;
}

