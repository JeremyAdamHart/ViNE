#pragma once

#include <openvr.h>
#include <Drawable.h>
#include <Texture.h>
#include <glm/glm.hpp>
#include <Object.h>

class VRController : public renderlib::Drawable {
public:
	VRController();
	VRController(vr::TrackedDeviceIndex_t index, vr::IVRSystem *vrSystem,
		vr::TrackedDevicePose_t pose, renderlib::TextureManager *texManager);

	vr::TrackedDeviceIndex_t index;
	vr::RenderModel_t* renderModel;

	enum {
		TRIGGER_AXIS = 0,
		TRACKPAD_AXIS,
		AXIS_COUNT
	};
	enum {
		TRACKPAD_BUTTON = 0,
		TRIGGER_BUTTON,
		GRIP_BUTTON,
		MENU_BUTTON,
		BUTTON_COUNT
	};

	glm::vec2 axes[AXIS_COUNT];
	bool buttons[BUTTON_COUNT];
	bool trackpadTouched;

	void updatePose(const vr::TrackedDevicePose_t &pose);
	void updateState(const vr::VRControllerState_t &state);
	void loadModelMatrixOldOpenGL() const;
};

class VRSceneTransform :public renderlib::Object {
public:
	VRSceneTransform();
	VRSceneTransform(std::vector<VRController> *controllers);

	//Rotation modes
	enum {
		HANDLEBAR,
		ONE_HAND_PLUS_SCALE,
		ORIGIN_CONTROLLER,
		ORIGIN_MODEL
	};

	float scale;

	glm::vec3 velocity;
	glm::quat angularVelocity;
	int rotationMode;
	int rotationOrigin;

	void setPosition(glm::vec3 position);

	glm::mat4 getTransform() const override;
	void updateTransform(float deltaTime);
	bool multMatrixPreviewTransform(float modelScale);
	void multMatrixOldOpenGL();
	void linkControllers(std::vector<VRController> *newControllers);
private:
	std::vector<VRController> *controllers;
	std::vector<glm::vec3> lastPosition;
	std::vector<glm::quat> lastOrientation;
};