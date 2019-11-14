#include "VRWindow.h"

#include <iostream>

using namespace glm;
using namespace std;

#include "Drawable.h"
#include "SimpleGeometry.h"

#include "SimpleShader.h"
#include "simpleTexShader.h"
#include "BlinnPhongShader.h"
#include "AOShader.h"
#include "PosNormalShader.h"
#include "BlinnPhongShaderVR.h"

#include "ColorMat.h"
#include "ShadedMat.h"
#include "TextureMat.h"

#include "TrackballCamera.h"
#include "TextureCreation.h"
#include "Framebuffer.h"
#include "SimpleTexManager.h"
#include "MeshInfoLoader.h"
#include "ModelLoader.h"

//Painting
#include "StreamGeometry.h"
#include "ColorShader.h"
#include "BubbleShader.h"
#include "ColorSetMat.h"
#include "kd_tree.h"
#include "VolumeIO.h"
#include "UndoStack.h"
#include "ColorWheel.h"
#include "VRColorShader.h"
#include "BlinnPhongShaderVR.h"

#include "VRController.h"
#include "VRView.h"
#include "VRContext.h"
#include "VRDeviceManager.h"

//Screenshot
#pragma warning(disable:4996)
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb/stb_image_write.h>

#include <glm/gtc/matrix_transform.hpp>

const float PI = 3.14159265358979323846;

using namespace renderlib;

int gWindowWidth, gWindowHeight;

bool reloadShaders = false;

TrackballCamera cam(
	vec3(0, 0, -1), vec3(0, 0, 1),
	//	glm::perspective(90.f*3.14159f/180.f, 1.f, 0.1f, 3.f));
	mat4(1.f));

void cursorPositionCallback(GLFWwindow *window, double xpos, double ypos) {
	static vec2 lastPos = vec2(0.f, 0.f);

	int vp[4];
	glGetIntegerv(GL_VIEWPORT, vp);
	vec2 mousePos = vec2(float(xpos) / float(vp[2]),
		float(-ypos) / float(vp[3]))*2.f
		- vec2(1.f, 1.f);

	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
		vec2 diff = mousePos - lastPos;
		cam.trackballRight(-diff.x*3.14159f);
		cam.trackballUp(-diff.y*3.14159f);
	}

	lastPos = mousePos;
}

void windowResizeCallback(GLFWwindow *window, int wWidth, int wHeight) {
	gWindowWidth = wWidth;
	gWindowHeight = wHeight;
}

WindowManager::WindowManager() :
	window_width(800), window_height(800)
{
	glfwInit();
	window = createWindow(window_width, window_height,
		"You really should rename this");
	initGlad();

	glfwSwapInterval(0);

	initGL();
}

WindowManager::WindowManager(int width, int height, std::string name, glm::vec4 color) :
	window_width(width), window_height(height)
{
	glfwInit();
	window = createWindow(window_width, window_height, name);
	initGlad();

	glfwSwapInterval(0);

	initGL();
}

void printMat4(const mat4 &m) {
	printf("[%.2f, %.2f, %.2f, %.2f]\n[%.2f, %.2f, %.2f, %.2f]\n[%.2f, %.2f, %.2f, %.2f]\n[%.2f, %.2f, %.2f, %.2f]\n\n",
		m[0][0], m[0][1], m[0][2], m[0][3],
		m[1][0], m[1][1], m[1][2], m[1][3],
		m[2][0], m[2][1], m[2][2], m[2][3],
		m[3][0], m[3][1], m[3][2], m[3][3]);
}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (key == GLFW_KEY_SPACE && action == GLFW_RELEASE)
		reloadShaders = true;
}

void WindowManager::mainLoopNoAO() {
/*	glfwSetCursorPosCallback(window, cursorPositionCallback);

	vec3 points[6] = {
		//First triangle
		vec3(-0.5f, 0.5f, 0.f)*2.f,
		vec3(0.5f, 0.5f, 0.f)*2.f,
		vec3(0.5f, -0.5f, 0.f)*2.f,
		//Second triangle
		vec3(0.5f, -0.5f, 0.f)*2.f,
		vec3(-0.5f, -0.5f, 0.f)*2.f,
		vec3(-0.5f, 0.5f, 0.f)*2.f
	};

	vec2 coords[6] = {
		//First triangle
		vec2(1, 0.f),
		vec2(0.f, 0.f),
		vec2(0.f, 1.f),
		//Second triangle
		vec2(0.f, 1.f),
		vec2(1.f, 1.f),
		vec2(1.f, 0.f)
	};

	SimpleTexManager tm;

	if (vrDisplay == nullptr) {
		vr::VR_Shutdown();
		glfwTerminate();
		return;
	}

	Framebuffer fbWindow(window_width, window_height);
	unsigned int TEX_WIDTH = 800;
	unsigned int TEX_HEIGHT = 800;
	vrDisplay->GetRecommendedRenderTargetSize(&TEX_WIDTH, &TEX_HEIGHT);

	Framebuffer fbLeftEyeDraw = createNewFramebuffer(TEX_WIDTH, TEX_HEIGHT);
	Framebuffer fbRightEyeDraw = createNewFramebuffer(TEX_WIDTH, TEX_HEIGHT);
	Framebuffer fbLeftEyeRead = createNewFramebuffer(TEX_WIDTH, TEX_HEIGHT);
	Framebuffer fbRightEyeRead = createNewFramebuffer(TEX_WIDTH, TEX_HEIGHT);


	const int NUM_SAMPLES = 8;

	if (!fbLeftEyeDraw.addTexture(
		createTexture2DMulti(TEX_WIDTH, TEX_HEIGHT, &tm, NUM_SAMPLES),
		GL_COLOR_ATTACHMENT0) ||
		!fbLeftEyeDraw.addTexture(
			createDepthTextureMulti(TEX_WIDTH, TEX_HEIGHT, &tm, NUM_SAMPLES), GL_DEPTH_ATTACHMENT))
	{
		std::cout << "FBO creation failed" << endl;
	}
	if (!fbRightEyeDraw.addTexture(
		createTexture2DMulti(TEX_WIDTH, TEX_HEIGHT, &tm, NUM_SAMPLES),
		GL_COLOR_ATTACHMENT0) ||
		!fbRightEyeDraw.addTexture(
			createDepthTextureMulti(TEX_WIDTH, TEX_HEIGHT, &tm, NUM_SAMPLES), GL_DEPTH_ATTACHMENT))
	{
		std::cout << "FBO creation failed" << endl;
	}

	if (!fbLeftEyeRead.addTexture(
		createTexture2D(TEX_WIDTH, TEX_HEIGHT, &tm), GL_COLOR_ATTACHMENT0)) {
		std::cout << "FBO creation failed" << endl;
	}
	else if (!fbRightEyeRead.addTexture(
		createTexture2D(TEX_WIDTH, TEX_HEIGHT, &tm), GL_COLOR_ATTACHMENT0)) {
		std::cout << "FBO creation failed" << endl;
	}

	Viewport leftEyeView(window_width / 2, window_height);
	Viewport rightEyeView(window_width / 2, window_height, window_width / 2);

	//Parse tracked devices
	int headsetIndex = 0;
	vector<VRController> controllers;
	vr::TrackedDevicePose_t poses[vr::k_unMaxTrackedDeviceCount];

	for (int i = 0; i < vr::k_unMaxTrackedDeviceCount; i++) {
		vr::TrackedDeviceClass deviceClass = vrDisplay->GetTrackedDeviceClass(i);

		switch (deviceClass) {
		case vr::TrackedDeviceClass_HMD:
			headsetIndex = i;
			break;
		case vr::TrackedDeviceClass_Controller:
			controllers.push_back(VRController(i, vrDisplay, poses[i], &tm));
			break;
		}
	}

	VRCameraController vrCam(&poses[headsetIndex], vrDisplay);

	//Dragon
	auto dragonGeom = shared_ptr<ElementGeometry>(objToElementGeometry("models/dragon.obj"));
	Drawable dragon(
		dragonGeom,
		make_shared<ColorMat>(vec3(0.75f, 0.1f, 0.3f))
	);
	dragon.addMaterial(new ShadedMat(0.2f, 0.5f, 0.3f, 10.f));

	dragon.setPosition(vec3(1.f, 0, 0));
	dragon.setOrientation(angleAxis(-PI*0.5f, vec3(0.f, 1.f, 0.f)));

	auto sphereGeom = shared_ptr<ElementGeometry>(objToElementGeometry("models/icosphere.obj"));
	Drawable sphere(
		sphereGeom,
		make_shared<ColorMat>(vec3(0.1, 0.3f, 0.8f))
	);
	sphere.addMaterial(new ShadedMat(0.2f, 0.5f, 0.3f, 10.f));

	sphere.setPosition(vec3(1.f, 0, 0));

	//Squares for left and right views
	Drawable leftSquare(
		new SimpleTexGeometry(points, coords, 6, GL_TRIANGLES),
		new TextureMat(fbLeftEyeRead.getTexture(GL_COLOR_ATTACHMENT0)));

	Drawable rightSquare(
		new SimpleTexGeometry(points, coords, 6, GL_TRIANGLES),
		new TextureMat(fbRightEyeRead.getTexture(GL_COLOR_ATTACHMENT0)));

	SimpleTexShader texShader;
	SimpleShader shader;
	BlinnPhongShader bpShader;
	BlinnPhongShader bpTexShader(BPTextureUsage::TEXTURE);

	TrackballCamera savedCam = cam;

	vec3 lightPos(-100.f, 100.f, 100.f);

	fbLeftEyeDraw.use();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	bpShader.draw(cam, lightPos, dragon);

	fbWindow.use();

	vector<Drawable> drawables;

	auto objGeometry = shared_ptr<ElementGeometry>(objToElementGeometry("untrackedmodels/riccoSurface_take3.obj"));

	drawables.push_back(Drawable(objGeometry, make_shared<ShadedMat>(0.3, 0.4, 0.4, 10.f)));
	drawables[0].addMaterial(new ColorMat(vec3(1, 1, 1)));

	for (int i = 0; i < drawables.size(); i++) {
		drawables[i].setPosition(vec3(0, 0, -6.f));
		drawables[i].setScale(vec3(10.0));
	}

	vector<vec3> controllerPositions(controllers.size());

	quat perFrameRot = angleAxis(3.14159f / 90.f, vec3(0, 1, 0));

	//Velocity
	vec3 linearVelocity(0.f);
	quat angularVelocity = quat();

	vector<vec3> tempData (objGeometry->numElements(), vec3(0, 1, 0));

	while (!glfwWindowShouldClose(window)) {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//Get pose
		vr::VRCompositor()->WaitGetPoses(poses, vr::k_unMaxTrackedDeviceCount,
			NULL, 0);

		//Update camera
		vrCam.update();
		vrCam.setProjection(vrDisplay, 0.2f, 400.f);

		//Update controllers
		vector<int> triggersPressed;
		for (int i = 0; i < controllers.size(); i++) {
			vr::VRControllerState_t state;
			vr::TrackedDevicePose_t pose;
			if (!vrDisplay->GetControllerStateWithPose(
				vr::VRCompositor()->GetTrackingSpace(),
				controllers[i].index,
				&state,
				sizeof(vr::VRControllerState_t),
				&pose))
			{
				printf("Error reading controller state\n");
			}

			controllers[i].updatePose(poses[controllers[i].index]);
			controllers[i].updateState(state);
			if (controllers[i].axes[VRController::TRIGGER_AXIS].x > 0.5f)
				triggersPressed.push_back(i);
		}

		vec3 positionTransform(0.f);
		quat orientationTransform;

		bool updatePositions = true;

		switch (triggersPressed.size()) {
		case 1:
		{
			vec3 lastPos = controllerPositions[triggersPressed[0]];
			positionTransform = controllers[triggersPressed[0]].getPos() - lastPos;
			linearVelocity = positionTransform;
			break;
		}
		case 2:
		{
			vec3 axisA = normalize(controllerPositions[triggersPressed[0]]
				- controllerPositions[triggersPressed[1]]);
			vec3 axisB = normalize(controllers[triggersPressed[0]].getPos()
				- controllers[triggersPressed[1]].getPos());
			vec3 rotAxis = cross(axisA, axisB);
			if (length(rotAxis) > 0.0001) {
				float angle = asin(length(rotAxis));
				orientationTransform = angleAxis(angle, normalize(rotAxis));
				angularVelocity = orientationTransform;
			}
			else
				updatePositions = false;
			break;

		}
		default:
			for (int i = 0; i < drawables.size(); i++) {
				quat orientation = drawables[i].getOrientationQuat();
				//				drawables[i].setOrientation(normalize(angularVelocity*orientation));
				drawables[i].setPosition(drawables[i].getPos() + linearVelocity);
			}

			//angularVelocity = slerp(angularVelocity, quat(), 0.002f);
			linearVelocity *= 0.99f;
		}

		if (updatePositions) {
			for (int i = 0; i < controllerPositions.size(); i++) {
				controllerPositions[i] = controllers[i].getPos();
			}
		}


		//Update model
		for (int i = 0; i < drawables.size(); i++) {
			quat orientation = drawables[i].getOrientationQuat();
			drawables[i].setOrientation(normalize(orientationTransform*orientation));
			drawables[i].setPosition(drawables[i].getPos() + positionTransform);

		}

		glEnable(GL_MULTISAMPLE);
		glClearColor(0.f, 0.f, 0.f, 1.f);

		dragon.setPosition(0.5f*vrCam.leftEye.getPosition()
			+ 0.5f*vrCam.rightEye.getPosition()
			+ vec3(0, 2, 0));

		//Draw left eye
		fbLeftEyeDraw.use();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		//		bpShader.draw(vrCam.leftEye, lightPos, dragon);
		for (int i = 0; i < controllers.size(); i++)
			bpTexShader.draw(vrCam.leftEye, lightPos, controllers[i]);

		for (int i = 0; i < drawables.size(); i++) {
			if (drawables[i].getMaterial(TextureMat::id) != nullptr) {
				bpTexShader.draw(vrCam.leftEye, lightPos, drawables[i]);
			}
			else
				bpShader.draw(vrCam.leftEye, lightPos, drawables[i]);
		}

		//Draw right eye
		fbRightEyeDraw.use();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		//		bpShader.draw(vrCam.rightEye, lightPos, dragon);
		for (int i = 0; i < controllers.size(); i++)
			bpTexShader.draw(vrCam.rightEye, lightPos, controllers[i]);
		for (int i = 0; i < drawables.size(); i++) {
			if (drawables[i].getMaterial(TextureMat::id) != nullptr) {
				bpTexShader.draw(vrCam.rightEye, lightPos, drawables[i]);
			}
			else
				bpShader.draw(vrCam.rightEye, lightPos, drawables[i]);
		}

		blit(fbLeftEyeDraw, fbLeftEyeRead);
		blit(fbRightEyeDraw, fbRightEyeRead);

		glDisable(GL_MULTISAMPLE);

		//Draw window
		fbWindow.use();
		leftEyeView.use();
		glClearColor(1.0, 1.0, 1.0, 1.0);
		texShader.draw(cam, leftSquare);

		rightEyeView.use();
		glClearColor(1.0, 1.0, 1.0, 1.0);
		texShader.draw(cam, rightSquare);

		//Draw headset
		if (vrDisplay) {
			vr::Texture_t leftTexture = {
				(void*)(uintptr_t)GLuint(fbLeftEyeRead.getTexture(GL_COLOR_ATTACHMENT0).getID()),
				vr::TextureType_OpenGL, vr::ColorSpace_Gamma };
			vr::Texture_t rightTexture = {
				(void*)(uintptr_t)GLuint(fbRightEyeRead.getTexture(GL_COLOR_ATTACHMENT0).getID()),
				vr::TextureType_OpenGL, vr::ColorSpace_Gamma };

			vr::VRCompositor()->Submit(vr::Eye_Left, &leftTexture);
			vr::VRCompositor()->Submit(vr::Eye_Right, &rightTexture);
		}

		checkGLErrors("Buffer overflow?");

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwTerminate();
	vr::VR_Shutdown();

	*/
}

////////////////////////////////////////////
// Support structs and functions for KDTree
////////////////////////////////////////////
struct IndexVec3 {
	size_t index;
	vec3 point;
	IndexVec3(size_t index, vec3 point) :index(index), point(point) {}
	float operator[](size_t index) const { return point[index]; }
	float &operator[](size_t index) { return point[index]; }
	IndexVec3 operator-(IndexVec3 p) const {
		p.point = point - p.point;
		p.index = -1;
		return p;
	}
};

float distanceSquared(IndexVec3 const &a, IndexVec3 const &b) {
	vec3 diff = a.point - b.point;
	return dot(diff, diff);
}

namespace spatial {
template<> constexpr uint16_t dimensions<IndexVec3>() { return 3; }
}

//Kalaxy - https://stackoverflow.com/questions/485525/round-for-float-in-c/4660122#4660122
int round_int(float val) {
	return (val > 0.f) ? (val + 0.5f) : (val - 0.5f);
}

float makeAnglePositive(float theta) {
	return mod(mod(theta, 2.f*PI) + 2.f*PI, 2.f*PI);
}

//Red at top, green at bottom right, blue at bottom left
vec3 angleToColor(float theta) {
	float u = mod(mod(theta / (2.f*PI), 1.f) + 1.f, 1.f);	//Positive modulo 1
	vec3 color(1, 1, 1);
	if (u < 1.f / 3.f) {
		float v = 3.f*u;
		color = (1 - v)*vec3(1, 0, 0) + v*vec3(0, 1, 0);
	} else if(u < 2.f/3.f){
		float v = 3.f*u - 1.f;
		color = (1 - v)*vec3(0, 1, 0) + v*vec3(0, 0, 1);
	} else {
		float v = 3.f*u - 2.f;
		color = (1 - v)*vec3(0, 0, 1) + v*vec3(1, 0, 0);
	}

	return normalize(color);
}

int axisToIndex(vec2 axis, int numIndices) {
	float theta = fmod(fmod(atan2(axis.y, axis.x), 2.f*PI) + 2.f*PI, 2.f*PI);
	float u = theta / (2.f*PI);
	int index =  round_int(u*float(numIndices)) % numIndices;
	return index;
}

#include <assert.h>

enum : int {
	PAINT_CONTROL = VRSceneTransform::ACTION_COUNT,
	UNDO_CONTROL,
	REDO_CONTROL,
	COLOR_SELECT_CONTROL,
	COLOR_DISPLAY_CONTROL,
	SPHERE_SIZE_CONTROL,
	SPHERE_SIZE_TOUCH_CONTROL,
	SPHERE_DISPLAY_CONTROL,
	SCREENSHOT_CONTROL,
	SAVE_VIEW_CONTROL
};

void setControllerBindingsOculusTouch(VRControllerInterface *input, VRControllerHand hand) {
	input->assignButton(VRSceneTransform::TRANSFORM_CONTROL, vr::k_EButton_Grip);
	input->assignAxis(PAINT_CONTROL, vr::k_EButton_SteamVR_Trigger);
	if (hand == VRControllerHand::LEFT) {
		input->assignButton(UNDO_CONTROL, OculusTouch_EButton_X);
		input->assignAxis(COLOR_SELECT_CONTROL, OculusTouch_EJoystick);
		input->assignTouch(COLOR_DISPLAY_CONTROL, OculusTouch_EJoystick);
		input->assignButton(SPHERE_DISPLAY_CONTROL, OculusTouch_ETrigger);

	}
	else {
		input->assignButton(REDO_CONTROL, OculusTouch_EButton_A);
		input->assignAxis(SPHERE_SIZE_CONTROL, OculusTouch_EJoystick);
		input->assignTouch(SPHERE_DISPLAY_CONTROL, OculusTouch_EJoystick);
		input->assignButton(SPHERE_DISPLAY_CONTROL, OculusTouch_ETrigger);
		input->assignTouch(SPHERE_SIZE_TOUCH_CONTROL, OculusTouch_EJoystick);
	}
}

void setControllerBindingsVive(VRControllerInterface *input, VRControllerHand hand) {
	input->assignButton(VRSceneTransform::TRANSFORM_CONTROL, vr::k_EButton_Grip);
	input->assignAxis(PAINT_CONTROL, vr::k_EButton_SteamVR_Trigger);
	if (hand == VRControllerHand::LEFT) {
		input->assignButton(UNDO_CONTROL, vr::k_EButton_ApplicationMenu);
		input->assignAxis(COLOR_SELECT_CONTROL, vr::k_EButton_SteamVR_Touchpad);
		input->assignTouch(COLOR_DISPLAY_CONTROL, vr::k_EButton_SteamVR_Touchpad);
		input->assignButton(SPHERE_DISPLAY_CONTROL, vr::k_EButton_SteamVR_Trigger);
//		input->assignButton(SCREENSHOT_CONTROL, vr::k_EButton_SteamVR_Touchpad);

	} else {
		input->assignButton(REDO_CONTROL, vr::k_EButton_ApplicationMenu);
		input->assignAxis(SPHERE_SIZE_CONTROL, vr::k_EButton_SteamVR_Touchpad);
		input->assignTouch(SPHERE_DISPLAY_CONTROL, vr::k_EButton_SteamVR_Touchpad);
		input->assignButton(SPHERE_DISPLAY_CONTROL, vr::k_EButton_SteamVR_Trigger);
		input->assignTouch(SPHERE_SIZE_TOUCH_CONTROL, vr::k_EButton_SteamVR_Touchpad);
		input->assignButton(SAVE_VIEW_CONTROL, vr::k_EButton_SteamVR_Touchpad);
	}
}

void setControllerBindingsWindows(VRControllerInterface *input, VRControllerHand hand) {
	input->assignButton(VRSceneTransform::TRANSFORM_CONTROL, vr::k_EButton_Grip);
	input->assignAxis(PAINT_CONTROL, Windows_ETrigger);
	if (hand == VRControllerHand::LEFT) {
		input->assignButton(UNDO_CONTROL, Windows_EButton_ApplicationMenu);
		input->assignAxis(COLOR_SELECT_CONTROL, Windows_EButton_Touchpad);
		input->assignTouch(COLOR_DISPLAY_CONTROL, Windows_EButton_Touchpad);
		input->assignButton(SPHERE_DISPLAY_CONTROL, Windows_ETrigger);

	}
	else {
		input->assignButton(REDO_CONTROL, Windows_EButton_ApplicationMenu);
		input->assignAxis(SPHERE_SIZE_CONTROL, Windows_EButton_Touchpad);
		input->assignTouch(SPHERE_DISPLAY_CONTROL, Windows_EButton_Touchpad);
		input->assignButton(SPHERE_DISPLAY_CONTROL, Windows_ETrigger);
		input->assignTouch(SPHERE_SIZE_TOUCH_CONTROL, Windows_EButton_Touchpad);
	}
}

void setBindings(VRControllerType type, VRControllerInterface *interface, VRControllerHand hand) {
	if (type == VRControllerType::VIVE) {
		setControllerBindingsVive(interface, hand);
	}
	else if (type == VRControllerType::OCULUS_TOUCH) {
		setControllerBindingsOculusTouch(interface, hand);
	}
	else if (type == VRControllerType::WINDOWS) {
		setControllerBindingsWindows(interface, hand);
	}
	else {
		printf("Error: Unknown controller model - Using Vive controls as default\n");
		setControllerBindingsVive(interface, hand);
	}
}

struct ControllerReferenceFilepaths {
	char* trackpadFrame;
	char* drawPosition;
	char* grabPosition;

	ControllerReferenceFilepaths(VRControllerType type):trackpadFrame(nullptr), drawPosition(nullptr), grabPosition(nullptr) {
		switch (type) {
		case VRControllerType::VIVE:
		case VRControllerType::UNKNOWN:
			trackpadFrame = "models/controllerTrackpadFrame.obj";
			drawPosition = "models/ViveDrawPosition.obj";
			grabPosition = "models/ViveGrabPosition.obj";
			break;
		case VRControllerType::WINDOWS:
			trackpadFrame = "models/WMRTrackpadFrame.obj";
			drawPosition = "models/WMRDrawPosition.obj";
			grabPosition = "models/WMRGrabPosition.obj";
			break;
		case VRControllerType::OCULUS_TOUCH:
			trackpadFrame = "models/OculusTouchTrackpadFrameLeft.obj";
			drawPosition = "models/OculusTouchDrawPosition.obj";
			grabPosition = "models/OculusTouchGrabPosition.obj";
			break;
		}
	}
};

struct Sphere {
	vec3 pos;
	float radius;
};

Sphere getBoundingSphere(const vector<vec3> &vertices) {
	vec3 averagePos = vec3(0.f);

	for (int i = 0; i < vertices.size(); i++) {
		averagePos += vertices[i];
	}
	averagePos /= float(vertices.size());

	float maxDist = 0.f;

	for (int i = 0; i < vertices.size(); i++) {
		maxDist = std::max(length(averagePos - vertices[i]), maxDist);
	}

	return { averagePos, maxDist };
}

int getNumberKeyPressed(GLFWwindow* window) {
	for (int i = 0; i < 10; i++) {
		if (glfwGetKey(window, GLFW_KEY_0 + i) == GLFW_PRESS) {
			return i;
		}
	}

	return 0;
}

vec3 closestPointOnLine(vec3 a, vec3 b, vec3 point) {
	float u = dot(b - a, point - a) / dot(b - a, b - a);
	if (u < 0)
		return a;
	else if (u > 1)
		return b;
	else
		return u*(b - a) + a;
}

vec3 closestPointOnTriangle(vec3 a, vec3 b, vec3 c, vec3 point) {
	mat2x3 A{ b - a, c - a };
	vec2 uv = inverse(transpose(A)*A)*transpose(A)*point;
	
	if (uv.x >= 0 && uv.y >= 0 && uv.x <= 1 && uv.y <= 1 && uv.x + uv.y < 1)
		return a + uv.x*(b - a) + uv.y*(c - a);

	vec3 closestAB = closestPointOnLine(a, b, point);
	vec3 closestBC = closestPointOnLine(b, c, point);
	vec3 closestCA = closestPointOnLine(c, a, point);

	float distAB = dot(closestAB - point, closestAB - point);
	float distBC = dot(closestBC - point, closestBC - point);
	float distCA = dot(closestCA - point, closestCA - point);

	if (distAB <= distBC && distAB <= distCA)
		return closestAB;
	else if (distBC <= distCA)
		return closestBC;
	else
		return closestCA;

}

std::pair<float, float> getClosestAndFurthestDistanceToConvexHull(vec3 point, vec3* hullPoints, unsigned int pointNum, unsigned int* hullFaces, unsigned int faceNum) {
	float closestDistance = std::numeric_limits<float>::max();
	float furthestDistance = -std::numeric_limits<float>::max();
	
	for (int i = 0; i < faceNum; i++) {
		vec3 a = hullPoints[hullFaces[3 * i]];
		vec3 b = hullPoints[hullFaces[3 * i + 1]];
		vec3 c = hullPoints[hullFaces[3 * i + 2]];

		vec3 newClosest = closestPointOnTriangle(a, b, c, point);
		
		closestDistance = std::min(closestDistance, dot(newClosest - point, newClosest - point));
	}

	for (int i = 0; i < pointNum; i++) {
		furthestDistance = std::max(furthestDistance, dot(hullPoints[i] - point, hullPoints[i] - point));
	}

	return{ sqrt(closestDistance), sqrt(furthestDistance) };
}

/*
Shader class
*/
constexpr int cMax(int a, int b) {
	return (a > b) ? a : b;
}

enum {
	VP_MATRIX_LOCATION = ShadedMat::COUNT
	+ cMax(int(TextureMat::COUNT), int(ColorMat::COUNT)),
	M_MATRIX_LOCATION,
	CAMERA_POS_LOCATION,
	LIGHT_POS_LOCATION,
	COUNT
};

static vector<pair<GLenum, string>> marchingCubeShaders{
	{ GL_VERTEX_SHADER, "shaders/marchingCubeShader.vert" },
	{ GL_FRAGMENT_SHADER, "shaders/marchingCubeShader.frag" }
};

class MarchingCubeBPShader : public ShaderT<ShadedMat, ColorMat> {

//TEST SHADER vv
	MarchingCubeBPShader::MarchingCubeBPShader(bool usingTexture) :
		ShaderT<ShadedMat, ColorMat>(marchingCubeShaders, {},
		{ "ka", "kd", "ks", "alpha", (usingTexture) ? "colorTexture" : "color",
			"view_projection_matrix",  "model_matrix", "camera_position", "lightPos" })
	{}

	void MarchingCubeBPShader::draw(const Camera &cam, vec3 lightPos,
	Drawable &obj)
	{
	glUseProgram(programID);

	mat4 vp_matrix = cam.getProjectionMatrix()*cam.getCameraMatrix();
	mat4 m_matrix = obj.getTransform();
	vec3 camera_pos = cam.getPosition();

	loadMaterialUniforms(obj);
	glUniformMatrix4fv(uniformLocations[VP_MATRIX_LOCATION], 1, false, &vp_matrix[0][0]);
	glUniformMatrix4fv(uniformLocations[M_MATRIX_LOCATION], 1, false, &m_matrix[0][0]);
	glUniform3f(uniformLocations[CAMERA_POS_LOCATION], camera_pos.x, camera_pos.y, camera_pos.z);
	glUniform3f(uniformLocations[LIGHT_POS_LOCATION],
		lightPos.x, lightPos.y, lightPos.z);
	obj.getGeometry().drawGeometry();
	glUseProgram(0);
	}
};

void WindowManager::paintingLoop(const char* loadedFile, const char* savedFile, int sampleNumber) {
	glfwSetCursorPosCallback(window, cursorPositionCallback);
	glfwSetWindowSizeCallback(window, windowResizeCallback);

	SimpleTexManager tm;
	VRContext vrContext(&tm);
	//IndexedViewportVRContext vrContext(&tm);

	if (vrContext.vrSystem == nullptr) {
		vr::VR_Shutdown();
		glfwTerminate();
		return;
	}

	const int FRAMES_PER_SECOND = 90;

	//Load model
	MeshInfoLoader minfo;
	vector<unsigned char> colors;	// (minfo.vertices.size(), 0);
	string objName;
	string savedFilename;
	if (hasExtension(loadedFile, ".obj")) {
		minfo.loadModel(loadedFile);
		
		colors.resize(minfo.vertices.size(), 0);
		objName = loadedFile;
		if (!hasExtension(savedFile, ".clr"))
			savedFilename = findFilenameVariation(
				"saved/" + swapExtension(getFilename(loadedFile), "clr"));
		else
			savedFilename = savedFile;
	}
	else if (hasExtension(loadedFile, ".ply")) {
		minfo.loadModelPly(loadedFile);
		colors.resize(minfo.vertices.size(), 0);
		objName = loadedFile;
		if (!hasExtension(savedFile, ".clr"))
			savedFilename = findFilenameVariation(
				"saved/" + swapExtension(getFilename(loadedFile), "clr"));
		else
			savedFilename = savedFile;
	}
	else {
		loadVolume(loadedFile, &minfo, &colors, &objName);
		savedFilename = savedFile;
	}

	Sphere boundingSphere = getBoundingSphere(minfo.vertices);

	printf("Number of vertices: %d\nNumber of faces: %d\n", minfo.vertices.size(), minfo.indices.size() / 3);

	vec3 points[6] = {
		//First triangle
		vec3(-0.5f, 0.5f, 0.f)*2.f,
		vec3(0.5f, -0.5f, 0.f)*2.f,
		vec3(0.5f, 0.5f, 0.f)*2.f,
		//Second triangle
		vec3(0.5f, -0.5f, 0.f)*2.f,
		vec3(-0.5f, 0.5f, 0.f)*2.f,
		vec3(-0.5f, -0.5f, 0.f)*2.f
	};

	vec2 coords[6] = {
		//First triangle
		vec2(1, 0.f),
		vec2(0.f, 1.f),
		vec2(0.f, 0.f),
		//Second triangle
		vec2(0.f, 1.f),
		vec2(1.f, 0.f),
		vec2(1.f, 1.f)
	};

	vec3 normals[6] = {
		vec3(0, 0, -1),
		vec3(0, 0, -1),
		vec3(0, 0, -1),
		vec3(0, 0, -1),
		vec3(0, 0, -1),
		vec3(0, 0, -1)
	};

	unsigned int SCREENSHOT_WIDTH = 6000;
	unsigned int SCREENSHOT_HEIGHT = 6000;

	Framebuffer fbWindow(window_width, window_height);
	gWindowWidth = window_width;
	gWindowHeight = window_height;
	unsigned int TEX_WIDTH = 800;
	unsigned int TEX_HEIGHT = 800;
	vrContext.vrSystem->GetRecommendedRenderTargetSize(&TEX_WIDTH, &TEX_HEIGHT);

	Framebuffer fbLeftEyeDraw = createFramebufferWithColorAndDepth(TEX_WIDTH, TEX_HEIGHT, &tm, sampleNumber);
	Framebuffer fbRightEyeDraw = createFramebufferWithColorAndDepth(TEX_WIDTH, TEX_HEIGHT, &tm, sampleNumber);

	Framebuffer fbScreenshotDraw = createFramebufferWithColorAndDepth(SCREENSHOT_WIDTH, SCREENSHOT_HEIGHT, &tm, 8);
	Framebuffer fbScreenshotRead = createFramebufferWithColorAndDepth(SCREENSHOT_WIDTH, SCREENSHOT_HEIGHT, &tm, 8);

	Viewport leftEyeView(window_width, window_height);
	Viewport rightEyeView(window_width / 2, window_height, window_width / 2);

	//Parse tracked devices
	VRDeviceManager<VRCameraController, VRController> devices(vrContext.vrSystem, &tm);
	VRController *controllers = devices.controllers;
	VRControllerType controllerType = controllers[0].type;

	setBindings(controllerType, &controllers[VRControllerHand::LEFT].input, VRControllerHand::LEFT);
	setBindings(controllerType, &controllers[VRControllerHand::RIGHT].input, VRControllerHand::RIGHT);

	bool controllerHasTrackpad = controllerType == VRControllerType::VIVE || controllerType == VRControllerType::WINDOWS || controllerType == VRControllerType::UNKNOWN;

	//Squares for left and right views
	Drawable leftSquare(
		new SimpleTexGeometry(points, coords, 6, GL_TRIANGLES),
		new TextureMat(vrContext.getTexture(vr::EVREye::Eye_Left)));

	Drawable rightSquare(
		new SimpleTexGeometry(points, coords, 6, GL_TRIANGLES),
		new TextureMat(vrContext.getTexture(vr::EVREye::Eye_Right)));

	SimpleTexShader texShader;
	BlinnPhongShader bpTexShader(BPTextureUsage::TEXTURE);
	BlinnPhongShader bpShader;
	BubbleShader bubbleShader;
	SimpleShader wireframeShade;

	TrackballCamera savedCam = cam;

	vec3 lightPos(-100.f, 100.f, 100.f);

	fbWindow.use();

	vector<Drawable> drawables;

	//Set up transparency
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	/////////////////////////	
	// STREAM GEOMETRY SETUP
	/////////////////////////
	//Generate color set
	vector<vec3> colorSet;
	colorSet = {
		vec3(1, 1, 1),
		vec3(1, 0, 0),
		vec3(1, 1, 0),
		vec3(0, 1, 0),
		vec3(0, 1, 1),
		vec3(0, 0, 1),
		vec3(1, 0, 1),
		vec3(1, 0.5, 0.25f)
	};

	colorSet = colorMapLoader("default.cmp");

	int COLOR_NUM = colorSet.size();

	VRColorShader colorShader(colorSet.size());

	enum {
		POSITION=0, NORMAL, COLOR	//Attribute indices
	 };

	//TEST -- Ground plane
	unsigned int groundIndices[6] = { 0, 2, 1, 4, 3, 5 };
	unsigned char groundColors[6] = { 0, 0, 0, 0, 0, 0 };
	auto groundGeom = make_shared<StreamGeometry<vec3, vec3, unsigned char>>(6, vector<char>{ false, false, false });
	groundGeom->loadElementArray(6, GL_STATIC_DRAW, groundIndices);
	groundGeom->loadBuffer<POSITION>(points);
	groundGeom->loadBuffer<NORMAL>(normals);
	groundGeom->loadBuffer<COLOR>(groundColors);
	Drawable groundPlane(
		groundGeom,
		make_shared<ColorSetMat>(colorSet)
	);
	groundPlane.addMaterial(new ShadedMat(0.3f, 0.4f, 0.4f, 50.f));
	groundPlane.orientation = glm::angleAxis(PI / 2.f, vec3(1, 0, 0));
	groundPlane.setScale(vec3(100.f));

	//END TEST -- Ground Plane

	auto streamGeometry = make_shared<StreamGeometry<vec3, vec3, unsigned char>>(minfo.vertices.size(),
	std::vector<char>({ false, false, true }));
	streamGeometry->loadElementArray(minfo.indices.size(), GL_STATIC_DRAW, minfo.indices.data());
	streamGeometry->loadBuffer<POSITION>(minfo.vertices.data());
	streamGeometry->loadBuffer<NORMAL>(minfo.normals.data());
	streamGeometry->loadBuffer<COLOR>(colors.data());

	drawables.push_back(Drawable(streamGeometry, make_shared<ShadedMat>(0.4, 0.7, 0.6, 10.f /*0.4, 0.5, 0.5, 10.f*/)));
	drawables[0].addMaterial(new ColorSetMat(colorSet));
	drawables[0].addMaterial(new ColorMat(vec3(1, 0, 0)));
	

	//Trackpad frame
	ControllerReferenceFilepaths controllerPath(controllerType);
//	const char* filePathTrackpadFrame = (controllerType == VRControllerType::VIVE)
//		? "models/controllerTrackpadFrame.obj" : "models/OculusTouchTrackpadFrameLeft.obj";
	MeshInfoLoader trackpadFrameObj(controllerPath.trackpadFrame);
	vec3 trackpadCenter(trackpadFrameObj.vertices[0]);
	vec3 trackpadBx(trackpadFrameObj.vertices[1] - trackpadCenter);
	vec3 trackpadBy(trackpadFrameObj.vertices[2] - trackpadCenter);
	vec3 trackpadNormal = normalize(cross(trackpadBx, trackpadBy));
	const float DIST_FROM_TPAD = 0.013f;
	const float COLOR_WHEEL_SCALE = 1.5f;

	//Draw position
	MeshInfoLoader drawPositionObj(controllerPath.drawPosition);
	vec3 drawPositionModelspace = drawPositionObj.vertices[0];

	//Grab position
	MeshInfoLoader grabPositionObj(controllerPath.grabPosition);
	vec3 grabPositionModelspace = grabPositionObj.vertices[0];

	//Trackpad geometry
	ColorWheel colorWheel(
		trackpadCenter + trackpadNormal*DIST_FROM_TPAD, 
		trackpadBx*COLOR_WHEEL_SCALE, 
		trackpadBy*COLOR_WHEEL_SCALE, 
		COLOR_NUM, 10);
	colorWheel.addMaterial(new ShadedMat(0.7f, 0.3f, 0.3f, 10.f));
	colorWheel.addMaterial(new ColorSetMat(colorSet));

	const float TRACKPAD_LIGHT_DIST = 0.5f;

	//Undo class
	const int MAX_UNDO = 5;
	UndoStack<unsigned char> undoStack(colors.data(), colors.size(), MAX_UNDO);

	//Drawing sphere
	unsigned char drawColor = 1;
	float sphereTransparency = 1.0f;
	float drawRadius = 0.05f;
	auto sphereGeom = shared_ptr<ElementGeometry>(objToElementGeometry("models/icosphere.obj"));
	auto sphereColorMat = make_shared<ColorMat>(colorSet[drawColor]);
	Drawable drawingSphere[2];
	for (int i = 0; i < 2; i++) {
		drawingSphere[i] = Drawable(sphereGeom, sphereColorMat);
		drawingSphere[i].setScale(vec3(drawRadius));
	}

	//Setup KDTree
	vector<IndexVec3> vertIndexPair;
	for (int i = 0; i < minfo.vertices.size(); i++) {
		vertIndexPair.push_back(IndexVec3(i, minfo.vertices[i]));
	}
	using namespace spatial;
	build_kdTree_inplace<dimensions<IndexVec3>()>(vertIndexPair.begin(), vertIndexPair.end());

	//Load convex hull
	string convexHullName = swapExtension(objName, ".hull");
	MeshInfoLoader convexHullMesh;
	convexHullMesh.loadModelPly(convexHullName.c_str());

	//Time tracking
	double frameTime = 0.f;
 	int frameTimeSamples = 0;
	double lastTime = glfwGetTime();

	vector<vec3> controllerPositions(2);

	VRSceneTransform sceneTransform;
	sceneTransform.setPosition(vec3(0.f, 1.f, -1.f));

	//Updating
	int counter = 0;
	float lastAngle_TrackpadRadius = 0.f; 
	bool released_TrackpadRadius = true;
	bool displaySphere[2] = { false, false };
	bool displayColorWheel = false;
	bool paintingButtonPressed[2] = { false, false };

	while (!glfwWindowShouldClose(window)) {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		if (gWindowWidth != window_width || gWindowHeight != window_height) {
			window_width = gWindowWidth;
			window_height = gWindowHeight;
			fbWindow.resize(window_width, window_height);
			leftEyeView.width = window_width;
			leftEyeView.height = window_height;
			rightEyeView.x = leftEyeView.width;
			rightEyeView.width = window_width - leftEyeView.width;
			rightEyeView.height = window_height;
		}

		displaySphere[0]= false;
		displaySphere[1] = false;

		devices.updatePose();
		devices.updateState(vrContext.vrSystem);

		vec2 trackpadDir[4] = { vec2(0, 1), vec2(1, 0), vec2(0, -1), vec2(-1, 0) };

		//Update colormap
		static bool updateMapButtonPressed = false;
		if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS && !updateMapButtonPressed) {
			updateMapButtonPressed = true;
			colorSet = colorMapLoader("default.cmp");
			dynamic_pointer_cast<ColorSetMat>(drawables[0].getMaterial(ColorSetMat::id))->colors = colorSet;
			dynamic_pointer_cast<ColorSetMat>(colorWheel.getMaterial(ColorSetMat::id))->colors = colorSet;
		}
		else if (glfwGetKey(window, GLFW_KEY_C) == GLFW_RELEASE)
			updateMapButtonPressed = false;

		static bool saveColoredPLYButton = false;
		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS && !saveColoredPLYButton) {
			printf("Saving colored ply\n");
			createPLYWithColors("coloredModel.ply", minfo.indices.data(), minfo.indices.size() / 3, minfo.vertices.data(), minfo.normals.data(),
				reinterpret_cast<unsigned char*>(streamGeometry->vboPointer<COLOR>()), colorSet.data(), minfo.vertices.size(), colorSet.size() - 1);
		}
		else if (glfwGetKey(window, GLFW_KEY_S) == GLFW_RELEASE)
			saveColoredPLYButton = false;

		//Get time
		static double lastTime = 0.f;
		double currentTime = glfwGetTime();
		sceneTransform.updateTransform(
			currentTime - lastTime,
			controllers[VRControllerHand::LEFT],
			controllers[VRControllerHand::RIGHT],
			grabPositionModelspace);
		lastTime = currentTime;

		const float MIN_TILT = (controllerHasTrackpad) ? 0.3f : 0.1f;	//Minimum offset from center for trackpads and joysticks

		//Change color based on axis
		if (controllers[VRControllerHand::LEFT].input.getActivation(COLOR_DISPLAY_CONTROL) ||
			(!controllerHasTrackpad && 
			length(controllers[VRControllerHand::LEFT].input.getAxis(COLOR_SELECT_CONTROL)) > MIN_TILT))
		{
			vec2 axis = controllers[VRControllerHand::LEFT].input.getAxis(COLOR_SELECT_CONTROL);
			displayColorWheel = true;
			colorWheel.thumbPos(axis);
			if (controllerHasTrackpad || length(axis) > MIN_TILT) {
				//			vec2 axis = controllers[0].axes[VRController::TRACKPAD_AXIS];
				drawColor = axisToIndex(axis, COLOR_NUM);
				sphereColorMat->color = vec4(colorSet[drawColor], sphereTransparency);
				colorWheel.selectColor(drawColor);
			}
		}
		else {
			colorWheel.selectColor(COLOR_NUM);		//Unset value
			displayColorWheel = false;
			colorWheel.thumbPos(vec2(20.f, 20.f));
		}
		//Change radius based on axis
		const float SCALE_PER_ROTATION = 2.f;
		const float MIN_DRAW_RADIUS = 0.01f;
		const float MAX_DRAW_RADIUS = 0.2f;


		if ((controllers[VRControllerHand::RIGHT].input.getActivation(SPHERE_SIZE_TOUCH_CONTROL)
			+ (length(controllers[VRControllerHand::RIGHT].input.getAxis(SPHERE_SIZE_CONTROL)) > MIN_TILT)
			+ (!controllerHasTrackpad)) >= 2)
		{
			
			vec2 axis = controllers[VRControllerHand::RIGHT].input.getAxis(SPHERE_SIZE_CONTROL);
			float currentAngle = atan2(axis.y, axis.x);

			if (controllerHasTrackpad) {
				if (released_TrackpadRadius) {
					lastAngle_TrackpadRadius = currentAngle;
					released_TrackpadRadius = false;
				}
				else {
					//Find shortest angular distance from last to current position
					float savedCurrentAngle = currentAngle;
					float diffA = currentAngle - lastAngle_TrackpadRadius;
					if (currentAngle < lastAngle_TrackpadRadius) currentAngle += 2.f*PI;
					else lastAngle_TrackpadRadius += 2.f*PI;
					float diffB = currentAngle - lastAngle_TrackpadRadius;
					float diff = (abs(diffA) < abs(diffB)) ? diffA : diffB;
				
					drawRadius = glm::clamp(drawRadius*pow(SCALE_PER_ROTATION, -diff / (2.f*PI)), MIN_DRAW_RADIUS, MAX_DRAW_RADIUS);
					drawingSphere[0].setScale(vec3(drawRadius));
					drawingSphere[1].setScale(vec3(drawRadius));

					lastAngle_TrackpadRadius = savedCurrentAngle;
				}
			}
			else {
				float scaleChange = pow(2.f, axis.y/float(FRAMES_PER_SECOND/2));	//Sphere size doubles in half a second

				drawRadius = glm::clamp(drawRadius*scaleChange, MIN_DRAW_RADIUS, MAX_DRAW_RADIUS);
				drawingSphere[0].setScale(vec3(drawRadius));
				drawingSphere[1].setScale(vec3(drawRadius));
			}
			displaySphere[1] = true;
		}
		else {
			released_TrackpadRadius = true;
		}

		//Update model
		for (int i = 0; i < drawables.size(); i++) {
			drawables[i].setOrientation(sceneTransform.getOrientationQuat());
			drawables[i].setPosition(sceneTransform.getPos());
			drawables[i].setScale(vec3(sceneTransform.scale));
		}

		//SEARCH KDTREE
		vector<IndexVec3> neighbours;
		for (int i = 0; i < 2; i++) {
			if (controllers[i].input.getActivation(SPHERE_DISPLAY_CONTROL)){
				vec3 pos = vec3(controllers[i].getTransform()*vec4(drawPositionModelspace, 1.f)); // TODO: write better code
				mat4 invrsTrans = inverse(sceneTransform.getTransform());
				pos = vec3(invrsTrans*vec4(pos, 1));
				float searchRadius = drawRadius / sceneTransform.scale;

				if(controllers[i].input.getScalar(PAINT_CONTROL) > 0.95f){
					if (paintingButtonPressed[i] == false) {
						paintingButtonPressed[i] = true;
						undoStack.startNewState();
					}
					kdTree_findNeighbours<dimensions<IndexVec3>()>(
						vertIndexPair.begin(), vertIndexPair.end(),
						IndexVec3(-1, pos),
						searchRadius*searchRadius,
						neighbours);
				}
				else {
					paintingButtonPressed[i] = false;
				}

				displaySphere[i] = true;		//TODO get rid of?
			}
			else {
				paintingButtonPressed == false;
			}
		}
		for (int i = 0; i < neighbours.size(); i++) {
			streamGeometry->modify<COLOR>(neighbours[i].index, drawColor);
			undoStack.modify(neighbours[i].index, drawColor);
		}

		streamGeometry->dump<COLOR>();
		streamGeometry->buffManager.endWrite();

		//Update color wheel position
		colorWheel.position = controllers[0].position;
		colorWheel.orientation = controllers[0].orientation;

		//Update sphere positions
		drawingSphere[0].position = vec3(controllers[0].getTransform()*vec4(drawPositionModelspace, 1.f));	//controllers[0].position;
		drawingSphere[1].position = vec3(controllers[1].getTransform()*vec4(drawPositionModelspace, 1.f));	//controllers[1].position;

		//Update bounding sphere on model and find fog bounds
		float closestPoint = std::numeric_limits<float>::max();
		float furthestPoint = -std::numeric_limits<float>::max();
		mat4 invModelMatrix = inverse(sceneTransform.getTransform());
		vec3 cameraPosition = vec3(invModelMatrix*vec4(devices.hmd.leftEye.getPosition(), 1));
		
		std::pair<float, float> distPair = getClosestAndFurthestDistanceToConvexHull(
			cameraPosition, 
			convexHullMesh.vertices.data(), 
			convexHullMesh.vertices.size(), 
			convexHullMesh.indices.data(), 
			convexHullMesh.indices.size() / 3);

		float fogDistance = distPair.first*sceneTransform.scale;
		float fogScale = (distPair.second - distPair.first)*0.5f*sceneTransform.scale;

		//Setup screenshot
		//Projection matrix for screenshots
		float aspectRatio = float(SCREENSHOT_WIDTH) / float(SCREENSHOT_HEIGHT);
		static mat4 screenshotProjection = perspective(radians(80.f), 1.f, 0.01f, 10.f);
		static mat4 savedProjection;
		static bool screenshotPressed = false;
		if (controllers[VRControllerHand::LEFT].input.getActivation(SCREENSHOT_CONTROL) && !screenshotPressed) {
			screenshotPressed = true;
			savedProjection = devices.hmd.rightEye.getProjectionMatrix();
			devices.hmd.rightEye.setProjectionMatrix(screenshotProjection);
		}
		else if(controllers[VRControllerHand::LEFT].input.getActivation(SCREENSHOT_CONTROL) && !screenshotPressed){
			devices.hmd.rightEye.setProjectionMatrix(savedProjection);
		}
		else {
			screenshotPressed = false;
		}

		//Load and save views
		static bool saveViewPressed = false;
		if (controllers[VRControllerHand::RIGHT].input.getActivation(SAVE_VIEW_CONTROL) && !saveViewPressed) {
			saveViewPressed = true;
			string filename = findFilenameVariation(string(loadedFile)+".view");
			vec3 cameraDir = vec3(devices.hmd.rightEye.getCameraMatrix()*vec4(0, 0, -1, 0));
			VRView view;
			view.generateView(devices.hmd.rightEye.getPosition(), cameraDir, &sceneTransform);
			view.scale = sceneTransform.scale;	//Should be part of function
			saveVRViewToFile(filename.c_str(), &view);

		}
		else if(!controllers[VRControllerHand::RIGHT].input.getActivation(SAVE_VIEW_CONTROL) && saveViewPressed)
			saveViewPressed = false;

		static bool loadViewPressed = false;
		int keyPressed = getNumberKeyPressed(window);
		if (keyPressed && !loadViewPressed) {
			loadViewPressed = true;
			vec3 cameraDir = vec3(devices.hmd.rightEye.getCameraMatrix()*vec4(0, 0, -1, 0));
			VRView view;
			if (loadVRViewFromFile((string(loadedFile) + to_string(keyPressed) + string(".view")).c_str(), &view)) {
				view.getViewFromCameraPositionAndOrientation(devices.hmd.rightEye.getPosition(), cameraDir, &sceneTransform);		//&drawables[0]);
				sceneTransform.scale = view.scale;		//Should be part of function
				sceneTransform.velocity = vec3(0);
				sceneTransform.angularVelocity = quat();
			}
		}
		else if (!keyPressed && loadViewPressed)
			loadViewPressed = false;

		////////////
		// DRAWING
		///////////
		glLineWidth(10.f);
		glEnable(GL_MULTISAMPLE);
		glClearColor(0.f, 0.f, 0.f, 0.f);

		//Draw left eye
		fbLeftEyeDraw.use();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		for (int i = 0; i < 2; i++)
			bpTexShader.draw(devices.hmd.leftEye, lightPos, controllers[i]);
		for (int i = 0; i < drawables.size(); i++) {
			colorShader.draw(devices.hmd.leftEye, lightPos, 
				fogScale, fogDistance, 
				vec3(0.02f, 0.04f, 0.07f), drawables[i]);		//Add lightPos and colorMat checking
		}
		if (displayColorWheel) {
			colorShader.draw(
				devices.hmd.leftEye,
				colorWheel.trackpadLightPosition(TRACKPAD_LIGHT_DIST),
				fogScale, 10.f,
				vec3(0.02f, 0.04f, 0.07f),
				colorWheel);
		}

		for (int i = 0; i < 2; i++) {
			if (displaySphere[i]) {
				glCullFace(GL_FRONT);
				bubbleShader.draw(devices.hmd.leftEye, drawingSphere[i]);
				glCullFace(GL_BACK);
				bubbleShader.draw(devices.hmd.leftEye, drawingSphere[i]);
			}
		}

		//Draw right eye -- Or screenshot
		if (!controllers[VRControllerHand::LEFT].input.getActivation(SCREENSHOT_CONTROL) || !screenshotPressed)
			fbRightEyeDraw.use();
		else
			fbScreenshotDraw.use();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		for (int i = 0; i < 2; i++)
			bpTexShader.draw(devices.hmd.rightEye, lightPos, controllers[i]);
		for (int i = 0; i < drawables.size(); i++) {
			colorShader.draw(devices.hmd.rightEye, lightPos, 
				fogScale, fogDistance, vec3(0.02f, 0.04f, 0.07f), drawables[i]);
		}
		if (displayColorWheel) {
			colorShader.draw(
				devices.hmd.rightEye,
				colorWheel.trackpadLightPosition(TRACKPAD_LIGHT_DIST), 
				fogScale, 10.f,
				vec3(0.02f, 0.04f, 0.07f),
				colorWheel);
		}
		
		for (int i = 0; i < 2; i++) {
			if (displaySphere[i]) {
				glCullFace(GL_FRONT);
				bubbleShader.draw(devices.hmd.rightEye, drawingSphere[i]);
				glCullFace(GL_BACK);
				bubbleShader.draw(devices.hmd.rightEye, drawingSphere[i]);
			}
		}


		glDisable(GL_BLEND);

		if (controllers[VRControllerHand::LEFT].input.getActivation(SCREENSHOT_CONTROL) && screenshotPressed)
			blit(fbScreenshotDraw, fbScreenshotRead);

		glDisable(GL_MULTISAMPLE);

		//Draw window
		fbWindow.use();
		leftEyeView.use();
		glClearColor(1.0, 1.0, 1.0, 0.0);
		texShader.draw(cam, leftSquare);

		//Draw headset
		vrContext.submitFrame(fbLeftEyeDraw, vr::EVREye::Eye_Left);
		vrContext.submitFrame(fbRightEyeDraw, vr::EVREye::Eye_Right);

		glEnable(GL_BLEND);

		checkGLErrors("Buffer overflow?");

		//Write screenshot to file
		if (screenshotPressed && controllers[VRControllerHand::LEFT].input.getActivation(SCREENSHOT_CONTROL)) {
			string filename = findFilenameVariation("Screenshot.png");

			Texture rightTex = fbScreenshotRead.getTexture(GL_COLOR_ATTACHMENT0);

			size_t imageSize = rightTex.getWidth()*rightTex.getHeight() * 4;
			unsigned char *data = new unsigned char[imageSize];
			glGetTextureImage(rightTex.getID(), 0, GL_RGBA, GL_UNSIGNED_BYTE, imageSize, data);
			stbi_flip_vertically_on_write(true);
			stbi_write_png(filename.c_str(), rightTex.getWidth(), rightTex.getHeight(), 4, data, 0);

			delete[] data;

			fbWindow.use();
		}

		if (frameTimeSamples > 30) {
			double currentTime = glfwGetTime();
			frameTime = currentTime - lastTime;
//			cout << "Time per frame = " << frameTime/double(frameTimeSamples) << endl;
			frameTimeSamples = 0;
			lastTime = currentTime;
		}
		else {
			frameTimeSamples++;
		}

		static bool saveButtonPressed = false;
		if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && !saveButtonPressed) {
			if (saveVolume(savedFilename.c_str(), objName.c_str(), streamGeometry->vboPointer<COLOR>(), colors.size()))
				printf("Saved %s successfully\n", savedFilename.c_str());
			else {
				printf("Attempting fallback - Saving to fallback.clr...\n");
				if (saveVolume("fallback.clr", objName.c_str(), streamGeometry->vboPointer<COLOR>(), colors.size()))
					printf("Saved fallback.clr successfully\n");
			}
			saveButtonPressed = true;
		}
		else if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_RELEASE) {
			saveButtonPressed = false;
		}
		static bool undoButtonPressed = false;
		bool pressed = controllers[0].input.getActivation(UNDO_CONTROL);
		if (pressed == false && undoButtonPressed) {
			map<size_t, unsigned char> changes;
			undoStack.undo(&changes);
			for (auto &it : changes) {
				streamGeometry->modify<COLOR>(it.first, it.second);
			}
			streamGeometry->dump<COLOR>();
			streamGeometry->buffManager.endWrite();
			undoButtonPressed = false;
		} else if (pressed) {
			undoButtonPressed = true;
		}
		static bool redoButtonPressed = false;
		pressed = controllers[1].input.getActivation(REDO_CONTROL);
		if (pressed == false && redoButtonPressed) {
			map<size_t, unsigned char> changes;
			undoStack.redo(&changes);
			for (auto &it : changes) {
				streamGeometry->modify<COLOR>(it.first, it.second);
			}
			streamGeometry->dump<COLOR>();
			streamGeometry->buffManager.endWrite();
			redoButtonPressed = false;
		}
		else if (pressed) {
			redoButtonPressed = true;
		}

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwTerminate();
	vr::VR_Shutdown();
}

void WindowManager::paintingLoopIndexed(const char* loadedFile, const char* savedFile, int sampleNumber) {
	glfwSetCursorPosCallback(window, cursorPositionCallback);
	glfwSetWindowSizeCallback(window, windowResizeCallback);

	SimpleTexManager tm;
	IndexedViewportVRContext vrContext(&tm);

	if (vrContext.vrSystem == nullptr) {
		vr::VR_Shutdown();
		glfwTerminate();
		return;
	}

	const int FRAMES_PER_SECOND = 90;

	//Load model
	MeshInfoLoader minfo;
	vector<unsigned char> colors;	// (minfo.vertices.size(), 0);
	string objName;
	string savedFilename;
	if (hasExtension(loadedFile, ".obj")) {
		minfo.loadModel(loadedFile);

		colors.resize(minfo.vertices.size(), 0);
		objName = loadedFile;
		if (!hasExtension(savedFile, ".clr"))
			savedFilename = findFilenameVariation(
				"saved/" + swapExtension(getFilename(loadedFile), "clr"));
		else
			savedFilename = savedFile;
	}
	else if (hasExtension(loadedFile, ".ply")) {
		minfo.loadModelPly(loadedFile);
		colors.resize(minfo.vertices.size(), 0);
		objName = loadedFile;
		if (!hasExtension(savedFile, ".clr"))
			savedFilename = findFilenameVariation(
				"saved/" + swapExtension(getFilename(loadedFile), "clr"));
		else
			savedFilename = savedFile;
	}
	else {
		loadVolume(loadedFile, &minfo, &colors, &objName);
		savedFilename = savedFile;
	}

	Sphere boundingSphere = getBoundingSphere(minfo.vertices);

	printf("Number of vertices: %d\nNumber of faces: %d\n", minfo.vertices.size(), minfo.indices.size() / 3);

	vec3 points[6] = {
		//First triangle
		vec3(-0.5f, 0.5f, 0.f)*2.f,
		vec3(0.5f, -0.5f, 0.f)*2.f,
		vec3(0.5f, 0.5f, 0.f)*2.f,
		//Second triangle
		vec3(0.5f, -0.5f, 0.f)*2.f,
		vec3(-0.5f, 0.5f, 0.f)*2.f,
		vec3(-0.5f, -0.5f, 0.f)*2.f
	};

	vec2 coords[6] = {
		//First triangle
		vec2(1, 0.f),
		vec2(0.f, 1.f),
		vec2(0.f, 0.f),
		//Second triangle
		vec2(0.f, 1.f),
		vec2(1.f, 0.f),
		vec2(1.f, 1.f)
	};

	vec3 normals[6] = {
		vec3(0, 0, -1),
		vec3(0, 0, -1),
		vec3(0, 0, -1),
		vec3(0, 0, -1),
		vec3(0, 0, -1),
		vec3(0, 0, -1)
	};

	unsigned int SCREENSHOT_WIDTH = 6000;
	unsigned int SCREENSHOT_HEIGHT = 6000;

	Framebuffer fbWindow(window_width, window_height);
	gWindowWidth = window_width;
	gWindowHeight = window_height;
	unsigned int TEX_WIDTH = 800;
	unsigned int TEX_HEIGHT = 800;
	vrContext.vrSystem->GetRecommendedRenderTargetSize(&TEX_WIDTH, &TEX_HEIGHT);

	IndexedFramebuffer fbDraw = createIndexedFramebufferWithColorAndDepth(TEX_WIDTH*2, TEX_HEIGHT, &tm, sampleNumber);
	fbDraw.addViewport(TEX_WIDTH, TEX_HEIGHT, 0, 0);
	fbDraw.addViewport(TEX_WIDTH, TEX_HEIGHT, TEX_WIDTH, 0);
	IndexedFramebuffer fbScreenshotDraw = createIndexedFramebufferWithColorAndDepth(SCREENSHOT_WIDTH, SCREENSHOT_HEIGHT, &tm, 8);
	IndexedFramebuffer fbScreenshotRead = createIndexedFramebufferWithColorAndDepth(SCREENSHOT_WIDTH, SCREENSHOT_HEIGHT, &tm, 8);

	Viewport leftEyeView(window_width, window_height);
	Viewport rightEyeView(window_width / 2, window_height, window_width / 2);

	//Parse tracked devices
	VRDeviceManager<VRCameraController, VRController> devices(vrContext.vrSystem, &tm);
	VRController *controllers = devices.controllers;
	VRControllerType controllerType = controllers[0].type;

	setBindings(controllerType, &controllers[VRControllerHand::LEFT].input, VRControllerHand::LEFT);
	setBindings(controllerType, &controllers[VRControllerHand::RIGHT].input, VRControllerHand::RIGHT);

	bool controllerHasTrackpad = controllerType == VRControllerType::VIVE || controllerType == VRControllerType::WINDOWS || controllerType == VRControllerType::UNKNOWN;

	//Squares for left and right views
	Drawable windowSquare(
		new SimpleTexGeometry(points, coords, 6, GL_TRIANGLES),
		new TextureMat(vrContext.getTexture()));

	SimpleTexShader texShader;
//	BlinnPhongShader bpTexShader(BPTextureUsage::TEXTURE);
//	BlinnPhongShader bpShader;
//	BubbleShader bubbleShader;
	SimpleShader wireframeShade;
	
	//Binocular shaders
	BubbleShaderBin bubbleShader;
	BlinnPhongShaderVR bpShader;
	BlinnPhongTexShaderVR bpTexShader;

	TrackballCamera savedCam = cam;

	vec3 lightPos(-100.f, 100.f, 100.f);

	fbWindow.use();

	vector<Drawable> drawables;

	//Set up transparency
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	/////////////////////////	
	// STREAM GEOMETRY SETUP
	/////////////////////////
	//Generate color set
	vector<vec3> colorSet;
	colorSet = {
		vec3(1, 1, 1),
		vec3(1, 0, 0),
		vec3(1, 1, 0),
		vec3(0, 1, 0),
		vec3(0, 1, 1),
		vec3(0, 0, 1),
		vec3(1, 0, 1),
		vec3(1, 0.5, 0.25f)
	};

	colorSet = colorMapLoader("default.cmp");

	int COLOR_NUM = colorSet.size();

	VRColorShaderBin colorShader(colorSet.size());

	enum {
		POSITION = 0, NORMAL, COLOR	//Attribute indices
	};

	//TEST -- Ground plane
	unsigned int groundIndices[6] = { 0, 2, 1, 4, 3, 5 };
	unsigned char groundColors[6] = { 0, 0, 0, 0, 0, 0 };
	auto groundGeom = make_shared<StreamGeometry<vec3, vec3, unsigned char>>(6, vector<char>{ false, false, false });
	groundGeom->loadElementArray(6, GL_STATIC_DRAW, groundIndices);
	groundGeom->loadBuffer<POSITION>(points);
	groundGeom->loadBuffer<NORMAL>(normals);
	groundGeom->loadBuffer<COLOR>(groundColors);
	Drawable groundPlane(
		groundGeom,
		make_shared<ColorSetMat>(colorSet)
	);
	groundPlane.addMaterial(new ShadedMat(0.3f, 0.4f, 0.4f, 50.f));
	groundPlane.orientation = glm::angleAxis(PI / 2.f, vec3(1, 0, 0));
	groundPlane.setScale(vec3(100.f));

	//END TEST -- Ground Plane

	auto streamGeometry = make_shared<StreamGeometry<vec3, vec3, unsigned char>>(minfo.vertices.size(),
		std::vector<char>({ false, false, true }));
	streamGeometry->loadElementArray(minfo.indices.size(), GL_STATIC_DRAW, minfo.indices.data());
	streamGeometry->loadBuffer<POSITION>(minfo.vertices.data());
	streamGeometry->loadBuffer<NORMAL>(minfo.normals.data());
	streamGeometry->loadBuffer<COLOR>(colors.data());

	drawables.push_back(Drawable(streamGeometry, make_shared<ShadedMat>(0.4, 0.7, 0.6, 10.f /*0.4, 0.5, 0.5, 10.f*/)));
	drawables[0].addMaterial(new ColorSetMat(colorSet));
	drawables[0].addMaterial(new ColorMat(vec3(1, 0, 0)));


	//Trackpad frame
	ControllerReferenceFilepaths controllerPath(controllerType);
	//	const char* filePathTrackpadFrame = (controllerType == VRControllerType::VIVE)
	//		? "models/controllerTrackpadFrame.obj" : "models/OculusTouchTrackpadFrameLeft.obj";
	MeshInfoLoader trackpadFrameObj(controllerPath.trackpadFrame);
	vec3 trackpadCenter(trackpadFrameObj.vertices[0]);
	vec3 trackpadBx(trackpadFrameObj.vertices[1] - trackpadCenter);
	vec3 trackpadBy(trackpadFrameObj.vertices[2] - trackpadCenter);
	vec3 trackpadNormal = normalize(cross(trackpadBx, trackpadBy));
	const float DIST_FROM_TPAD = 0.013f;
	const float COLOR_WHEEL_SCALE = 1.5f;

	//Draw position
	MeshInfoLoader drawPositionObj(controllerPath.drawPosition);
	vec3 drawPositionModelspace = drawPositionObj.vertices[0];

	//Grab position
	MeshInfoLoader grabPositionObj(controllerPath.grabPosition);
	vec3 grabPositionModelspace = grabPositionObj.vertices[0];

	//Trackpad geometry
	ColorWheel colorWheel(
		trackpadCenter + trackpadNormal * DIST_FROM_TPAD,
		trackpadBx*COLOR_WHEEL_SCALE,
		trackpadBy*COLOR_WHEEL_SCALE,
		COLOR_NUM, 10);
	colorWheel.addMaterial(new ShadedMat(0.7f, 0.3f, 0.3f, 10.f));
	colorWheel.addMaterial(new ColorSetMat(colorSet));

	const float TRACKPAD_LIGHT_DIST = 0.5f;

	//Undo class
	const int MAX_UNDO = 5;
	UndoStack<unsigned char> undoStack(colors.data(), colors.size(), MAX_UNDO);

	//Drawing sphere
	unsigned char drawColor = 1;
	float sphereTransparency = 1.0f;
	float drawRadius = 0.05f;
	auto sphereGeom = shared_ptr<ElementGeometry>(objToElementGeometry("models/icosphere.obj"));
	auto sphereColorMat = make_shared<ColorMat>(colorSet[drawColor]);
	Drawable drawingSphere[2];
	for (int i = 0; i < 2; i++) {
		drawingSphere[i] = Drawable(sphereGeom, sphereColorMat);
		drawingSphere[i].setScale(vec3(drawRadius));
	}

	//Setup KDTree
	vector<IndexVec3> vertIndexPair;
	for (int i = 0; i < minfo.vertices.size(); i++) {
		vertIndexPair.push_back(IndexVec3(i, minfo.vertices[i]));
	}
	using namespace spatial;
	build_kdTree_inplace<dimensions<IndexVec3>()>(vertIndexPair.begin(), vertIndexPair.end());

	//Load convex hull
	string convexHullName = swapExtension(objName, ".hull");
	MeshInfoLoader convexHullMesh;
	convexHullMesh.loadModelPly(convexHullName.c_str());

	//Time tracking
	double frameTime = 0.f;
	int frameTimeSamples = 0;
	double lastTime = glfwGetTime();

	vector<vec3> controllerPositions(2);

	VRSceneTransform sceneTransform;
	sceneTransform.setPosition(vec3(0.f, 1.f, -1.f));

	//Updating
	int counter = 0;
	float lastAngle_TrackpadRadius = 0.f;
	bool released_TrackpadRadius = true;
	bool displaySphere[2] = { false, false };
	bool displayColorWheel = false;
	bool paintingButtonPressed[2] = { false, false };

	while (!glfwWindowShouldClose(window)) {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		if (gWindowWidth != window_width || gWindowHeight != window_height) {
			window_width = gWindowWidth;
			window_height = gWindowHeight;
			fbWindow.resize(window_width, window_height);
			leftEyeView.width = window_width;
			leftEyeView.height = window_height;
			rightEyeView.x = leftEyeView.width;
			rightEyeView.width = window_width - leftEyeView.width;
			rightEyeView.height = window_height;
		}

		displaySphere[0] = false;
		displaySphere[1] = false;

		devices.updatePose();
		devices.updateState(vrContext.vrSystem);

		vec2 trackpadDir[4] = { vec2(0, 1), vec2(1, 0), vec2(0, -1), vec2(-1, 0) };

		//Update colormap
		static bool updateMapButtonPressed = false;
		if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS && !updateMapButtonPressed) {
			updateMapButtonPressed = true;
			colorSet = colorMapLoader("default.cmp");
			dynamic_pointer_cast<ColorSetMat>(drawables[0].getMaterial(ColorSetMat::id))->colors = colorSet;
			dynamic_pointer_cast<ColorSetMat>(colorWheel.getMaterial(ColorSetMat::id))->colors = colorSet;
		}
		else if (glfwGetKey(window, GLFW_KEY_C) == GLFW_RELEASE)
			updateMapButtonPressed = false;

		static bool saveColoredPLYButton = false;
		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS && !saveColoredPLYButton) {
			printf("Saving colored ply\n");
			createPLYWithColors("coloredModel.ply", minfo.indices.data(), minfo.indices.size() / 3, minfo.vertices.data(), minfo.normals.data(),
				reinterpret_cast<unsigned char*>(streamGeometry->vboPointer<COLOR>()), colorSet.data(), minfo.vertices.size(), colorSet.size() - 1);
		}
		else if (glfwGetKey(window, GLFW_KEY_S) == GLFW_RELEASE)
			saveColoredPLYButton = false;

		//Get time
		static double lastTime = 0.f;
		double currentTime = glfwGetTime();
		sceneTransform.updateTransform(
			currentTime - lastTime,
			controllers[VRControllerHand::LEFT],
			controllers[VRControllerHand::RIGHT],
			grabPositionModelspace);
		lastTime = currentTime;

		const float MIN_TILT = (controllerHasTrackpad) ? 0.3f : 0.1f;	//Minimum offset from center for trackpads and joysticks

		//Change color based on axis
		if (controllers[VRControllerHand::LEFT].input.getActivation(COLOR_DISPLAY_CONTROL) ||
			(!controllerHasTrackpad &&
				length(controllers[VRControllerHand::LEFT].input.getAxis(COLOR_SELECT_CONTROL)) > MIN_TILT))
		{
			vec2 axis = controllers[VRControllerHand::LEFT].input.getAxis(COLOR_SELECT_CONTROL);
			displayColorWheel = true;
			colorWheel.thumbPos(axis);
			if (controllerHasTrackpad || length(axis) > MIN_TILT) {
				//			vec2 axis = controllers[0].axes[VRController::TRACKPAD_AXIS];
				drawColor = axisToIndex(axis, COLOR_NUM);
				sphereColorMat->color = vec4(colorSet[drawColor], sphereTransparency);
				colorWheel.selectColor(drawColor);
			}
		}
		else {
			colorWheel.selectColor(COLOR_NUM);		//Unset value
			displayColorWheel = false;
			colorWheel.thumbPos(vec2(20.f, 20.f));
		}
		//Change radius based on axis
		const float SCALE_PER_ROTATION = 2.f;
		const float MIN_DRAW_RADIUS = 0.01f;
		const float MAX_DRAW_RADIUS = 0.2f;


		if ((controllers[VRControllerHand::RIGHT].input.getActivation(SPHERE_SIZE_TOUCH_CONTROL)
			+ (length(controllers[VRControllerHand::RIGHT].input.getAxis(SPHERE_SIZE_CONTROL)) > MIN_TILT)
			+ (!controllerHasTrackpad)) >= 2)
		{

			vec2 axis = controllers[VRControllerHand::RIGHT].input.getAxis(SPHERE_SIZE_CONTROL);
			float currentAngle = atan2(axis.y, axis.x);

			if (controllerHasTrackpad) {
				if (released_TrackpadRadius) {
					lastAngle_TrackpadRadius = currentAngle;
					released_TrackpadRadius = false;
				}
				else {
					//Find shortest angular distance from last to current position
					float savedCurrentAngle = currentAngle;
					float diffA = currentAngle - lastAngle_TrackpadRadius;
					if (currentAngle < lastAngle_TrackpadRadius) currentAngle += 2.f*PI;
					else lastAngle_TrackpadRadius += 2.f*PI;
					float diffB = currentAngle - lastAngle_TrackpadRadius;
					float diff = (abs(diffA) < abs(diffB)) ? diffA : diffB;

					drawRadius = glm::clamp(drawRadius*pow(SCALE_PER_ROTATION, -diff / (2.f*PI)), MIN_DRAW_RADIUS, MAX_DRAW_RADIUS);
					drawingSphere[0].setScale(vec3(drawRadius));
					drawingSphere[1].setScale(vec3(drawRadius));

					lastAngle_TrackpadRadius = savedCurrentAngle;
				}
			}
			else {
				float scaleChange = pow(2.f, axis.y / float(FRAMES_PER_SECOND / 2));	//Sphere size doubles in half a second

				drawRadius = glm::clamp(drawRadius*scaleChange, MIN_DRAW_RADIUS, MAX_DRAW_RADIUS);
				drawingSphere[0].setScale(vec3(drawRadius));
				drawingSphere[1].setScale(vec3(drawRadius));
			}
			displaySphere[1] = true;
		}
		else {
			released_TrackpadRadius = true;
		}

		//Update model
		for (int i = 0; i < drawables.size(); i++) {
			drawables[i].setOrientation(sceneTransform.getOrientationQuat());
			drawables[i].setPosition(sceneTransform.getPos());
			drawables[i].setScale(vec3(sceneTransform.scale));
		}

		//SEARCH KDTREE
		vector<IndexVec3> neighbours;
		for (int i = 0; i < 2; i++) {
			if (controllers[i].input.getActivation(SPHERE_DISPLAY_CONTROL)) {
				vec3 pos = vec3(controllers[i].getTransform()*vec4(drawPositionModelspace, 1.f)); // TODO: write better code
				mat4 invrsTrans = inverse(sceneTransform.getTransform());
				pos = vec3(invrsTrans*vec4(pos, 1));
				float searchRadius = drawRadius / sceneTransform.scale;

				if (controllers[i].input.getScalar(PAINT_CONTROL) > 0.95f) {
					if (paintingButtonPressed[i] == false) {
						paintingButtonPressed[i] = true;
						undoStack.startNewState();
					}
					kdTree_findNeighbours<dimensions<IndexVec3>()>(
						vertIndexPair.begin(), vertIndexPair.end(),
						IndexVec3(-1, pos),
						searchRadius*searchRadius,
						neighbours);
				}
				else {
					paintingButtonPressed[i] = false;
				}

				displaySphere[i] = true;		//TODO get rid of?
			}
			else {
				paintingButtonPressed == false;
			}
		}
		for (int i = 0; i < neighbours.size(); i++) {
			streamGeometry->modify<COLOR>(neighbours[i].index, drawColor);
			undoStack.modify(neighbours[i].index, drawColor);
		}

		streamGeometry->dump<COLOR>();
		streamGeometry->buffManager.endWrite();

		//Update color wheel position
		colorWheel.position = controllers[0].position;
		colorWheel.orientation = controllers[0].orientation;

		//Update sphere positions
		drawingSphere[0].position = vec3(controllers[0].getTransform()*vec4(drawPositionModelspace, 1.f));	//controllers[0].position;
		drawingSphere[1].position = vec3(controllers[1].getTransform()*vec4(drawPositionModelspace, 1.f));	//controllers[1].position;

		//Update bounding sphere on model and find fog bounds
		float closestPoint = std::numeric_limits<float>::max();
		float furthestPoint = -std::numeric_limits<float>::max();
		mat4 invModelMatrix = inverse(sceneTransform.getTransform());
		vec3 cameraPosition = vec3(invModelMatrix*vec4(devices.hmd.leftEye.getPosition(), 1));

		std::pair<float, float> distPair = getClosestAndFurthestDistanceToConvexHull(
			cameraPosition,
			convexHullMesh.vertices.data(),
			convexHullMesh.vertices.size(),
			convexHullMesh.indices.data(),
			convexHullMesh.indices.size() / 3);

		float fogDistance = distPair.first*sceneTransform.scale;
		float fogScale = (distPair.second - distPair.first)*0.5f*sceneTransform.scale;

		//Setup screenshot
		//Projection matrix for screenshots
		float aspectRatio = float(SCREENSHOT_WIDTH) / float(SCREENSHOT_HEIGHT);
		static mat4 screenshotProjection = perspective(radians(80.f), 1.f, 0.01f, 10.f);
		static mat4 savedProjection;
		static bool screenshotPressed = false;
		if (controllers[VRControllerHand::LEFT].input.getActivation(SCREENSHOT_CONTROL) && !screenshotPressed) {
			screenshotPressed = true;
			savedProjection = devices.hmd.rightEye.getProjectionMatrix();
			devices.hmd.rightEye.setProjectionMatrix(screenshotProjection);
		}
		else if (controllers[VRControllerHand::LEFT].input.getActivation(SCREENSHOT_CONTROL) && !screenshotPressed) {
			devices.hmd.rightEye.setProjectionMatrix(savedProjection);
		}
		else {
			screenshotPressed = false;
		}

		//Load and save views
		static bool saveViewPressed = false;
		if (controllers[VRControllerHand::RIGHT].input.getActivation(SAVE_VIEW_CONTROL) && !saveViewPressed) {
			saveViewPressed = true;
			string filename = findFilenameVariation(string(loadedFile) + ".view");
			vec3 cameraDir = vec3(devices.hmd.rightEye.getCameraMatrix()*vec4(0, 0, -1, 0));
			VRView view;
			view.generateView(devices.hmd.rightEye.getPosition(), cameraDir, &sceneTransform);
			view.scale = sceneTransform.scale;	//Should be part of function
			saveVRViewToFile(filename.c_str(), &view);

		}
		else if (!controllers[VRControllerHand::RIGHT].input.getActivation(SAVE_VIEW_CONTROL) && saveViewPressed)
			saveViewPressed = false;

		static bool loadViewPressed = false;
		int keyPressed = getNumberKeyPressed(window);
		if (keyPressed && !loadViewPressed) {
			loadViewPressed = true;
			vec3 cameraDir = vec3(devices.hmd.rightEye.getCameraMatrix()*vec4(0, 0, -1, 0));
			VRView view;
			if (loadVRViewFromFile((string(loadedFile) + to_string(keyPressed) + string(".view")).c_str(), &view)) {
				view.getViewFromCameraPositionAndOrientation(devices.hmd.rightEye.getPosition(), cameraDir, &sceneTransform);		//&drawables[0]);
				sceneTransform.scale = view.scale;		//Should be part of function
				sceneTransform.velocity = vec3(0);
				sceneTransform.angularVelocity = quat();
			}
		}
		else if (!keyPressed && loadViewPressed)
			loadViewPressed = false;

		////////////
		// DRAWING
		///////////
		glLineWidth(10.f);
		glEnable(GL_MULTISAMPLE);
		glClearColor(0.f, 0.f, 0.f, 0.f);

		fbDraw.use();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		for (int i = 0; i < 2; i++)
			bpTexShader.draw(devices.hmd.leftEye, devices.hmd.rightEye, lightPos, controllers[i]);
		for (int i = 0; i < drawables.size(); i++) {
			colorShader.draw(devices.hmd.leftEye, devices.hmd.rightEye, lightPos,
				fogScale, fogDistance,
				vec3(0.02f, 0.04f, 0.07f), drawables[i]);		//Add lightPos and colorMat checking
		}
		if (displayColorWheel) {
			colorShader.draw(
				devices.hmd.leftEye,
				devices.hmd.rightEye,
				colorWheel.trackpadLightPosition(TRACKPAD_LIGHT_DIST),
				fogScale, 10.f,
				vec3(0.02f, 0.04f, 0.07f),
				colorWheel);
		}

		for (int i = 0; i < 2; i++) {
			if (displaySphere[i]) {
				glCullFace(GL_FRONT);
				bubbleShader.draw(devices.hmd.leftEye, devices.hmd.rightEye, drawingSphere[i]);
				glCullFace(GL_BACK);
				bubbleShader.draw(devices.hmd.leftEye, devices.hmd.rightEye, drawingSphere[i]);
			}
		}

		glDisable(GL_MULTISAMPLE);
		glDisable(GL_BLEND);

		//Draw window
		fbWindow.use();
		//leftEyeView.use();
		glClearColor(1.0, 1.0, 1.0, 0.0);
		texShader.draw(cam, windowSquare);

		//Draw headset
		vrContext.submitFrame(fbDraw);

		glEnable(GL_BLEND);

		checkGLErrors("Buffer overflow?");

		//Write screenshot to file
		if (screenshotPressed && controllers[VRControllerHand::LEFT].input.getActivation(SCREENSHOT_CONTROL)) {
			string filename = findFilenameVariation("Screenshot.png");

			Texture rightTex = fbScreenshotRead.getTexture(GL_COLOR_ATTACHMENT0);

			size_t imageSize = rightTex.getWidth()*rightTex.getHeight() * 4;
			unsigned char *data = new unsigned char[imageSize];
			glGetTextureImage(rightTex.getID(), 0, GL_RGBA, GL_UNSIGNED_BYTE, imageSize, data);
			stbi_flip_vertically_on_write(true);
			stbi_write_png(filename.c_str(), rightTex.getWidth(), rightTex.getHeight(), 4, data, 0);

			delete[] data;

			fbWindow.use();
		}

		if (frameTimeSamples > 30) {
			double currentTime = glfwGetTime();
			frameTime = currentTime - lastTime;
			//			cout << "Time per frame = " << frameTime/double(frameTimeSamples) << endl;
			frameTimeSamples = 0;
			lastTime = currentTime;
		}
		else {
			frameTimeSamples++;
		}

		static bool saveButtonPressed = false;
		if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && !saveButtonPressed) {
			if (saveVolume(savedFilename.c_str(), objName.c_str(), streamGeometry->vboPointer<COLOR>(), colors.size()))
				printf("Saved %s successfully\n", savedFilename.c_str());
			else {
				printf("Attempting fallback - Saving to fallback.clr...\n");
				if (saveVolume("fallback.clr", objName.c_str(), streamGeometry->vboPointer<COLOR>(), colors.size()))
					printf("Saved fallback.clr successfully\n");
			}
			saveButtonPressed = true;
		}
		else if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_RELEASE) {
			saveButtonPressed = false;
		}
		static bool undoButtonPressed = false;
		bool pressed = controllers[0].input.getActivation(UNDO_CONTROL);
		if (pressed == false && undoButtonPressed) {
			map<size_t, unsigned char> changes;
			undoStack.undo(&changes);
			for (auto &it : changes) {
				streamGeometry->modify<COLOR>(it.first, it.second);
			}
			streamGeometry->dump<COLOR>();
			streamGeometry->buffManager.endWrite();
			undoButtonPressed = false;
		}
		else if (pressed) {
			undoButtonPressed = true;
		}
		static bool redoButtonPressed = false;
		pressed = controllers[1].input.getActivation(REDO_CONTROL);
		if (pressed == false && redoButtonPressed) {
			map<size_t, unsigned char> changes;
			undoStack.redo(&changes);
			for (auto &it : changes) {
				streamGeometry->modify<COLOR>(it.first, it.second);
			}
			streamGeometry->dump<COLOR>();
			streamGeometry->buffManager.endWrite();
			redoButtonPressed = false;
		}
		else if (pressed) {
			redoButtonPressed = true;
		}

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwTerminate();
	vr::VR_Shutdown();
}


//Temporary testing
void WindowManager::mainLoop() {
/*
	//Test quaterions
	glfwSetCursorPosCallback(window, cursorPositionCallback);

	vec3 points[6] = {
		//First triangle
		vec3(-0.5f, 0.5f, 0.f)*2.f,
		vec3(0.5f, 0.5f, 0.f)*2.f,
		vec3(0.5f, -0.5f, 0.f)*2.f,
		//Second triangle
		vec3(0.5f, -0.5f, 0.f)*2.f,
		vec3(-0.5f, -0.5f, 0.f)*2.f,
		vec3(-0.5f, 0.5f, 0.f)*2.f
	};
	vec2 coords[6] = {
		//First triangle
		vec2(0, 1.f),
		vec2(1.f, 1.f),
		vec2(1.f, 0.f),
		//Second triangle
		vec2(1.f, 0.f),
		vec2(0.f, 0.f),
		vec2(0.f, 1.f)
	};

	vec2 coords2[6] = {
		//First triangle
		vec2(1, 0.f),
		vec2(0.f, 0.f),
		vec2(0.f, 1.f),
		//Second triangle
		vec2(0.f, 1.f),
		vec2(1.f, 1.f),
		vec2(1.f, 0.f)
	};

	SimpleTexManager tm;

	if (vrDisplay == nullptr) {
		vr::VR_Shutdown();
		glfwTerminate();
		return;
	}

	Framebuffer fbWindow(window_width, window_height);
	unsigned int TEX_WIDTH = 800;
	unsigned int TEX_HEIGHT = 800;
	vrDisplay->GetRecommendedRenderTargetSize(&TEX_WIDTH, &TEX_HEIGHT);
	//	glfwSetWindowSize(window, TEX_WIDTH * 2, TEX_HEIGHT);

	glfwSetKeyCallback(window, keyCallback);

	Framebuffer fbLeftEyeDraw = createNewFramebuffer(TEX_WIDTH, TEX_HEIGHT);
	Framebuffer fbRightEyeDraw = createNewFramebuffer(TEX_WIDTH, TEX_HEIGHT);
	Framebuffer fbLeftEyeRead = createNewFramebuffer(TEX_WIDTH, TEX_HEIGHT);
	Framebuffer fbRightEyeRead = createNewFramebuffer(TEX_WIDTH, TEX_HEIGHT);


	const int NUM_SAMPLES = 1;

	//TEST vv

	if (!fbLeftEyeDraw.addTexture(
		createTexture2DMulti(
			TexInfo(GL_TEXTURE_2D_MULTISAMPLE, { int(TEX_WIDTH), int(TEX_HEIGHT) }, 0, GL_RGB, GL_RGB16F, GL_FLOAT), &tm, NUM_SAMPLES),
		GL_COLOR_ATTACHMENT0) ||
		!fbLeftEyeDraw.addTexture(
			createDepthTextureMulti(TEX_WIDTH, TEX_HEIGHT, &tm, NUM_SAMPLES),
			GL_DEPTH_ATTACHMENT) ||
		!fbLeftEyeDraw.addTexture(
			createTexture2DMulti(
				TexInfo(GL_TEXTURE_2D_MULTISAMPLE, { int(TEX_WIDTH), int(TEX_HEIGHT) }, 0, GL_RGB, GL_RGB16F, GL_FLOAT), &tm, NUM_SAMPLES),
			GL_COLOR_ATTACHMENT1))
	{
		std::cout << "FBO creation failed" << endl;
	}

	if (!fbRightEyeDraw.addTexture(
		createTexture2DMulti(
			TexInfo(GL_TEXTURE_2D_MULTISAMPLE, { int(TEX_WIDTH), int(TEX_HEIGHT) }, 0, GL_RGB, GL_RGB16F, GL_FLOAT), &tm, NUM_SAMPLES),
		GL_COLOR_ATTACHMENT0) ||
		!fbRightEyeDraw.addTexture(
			createDepthTextureMulti(TEX_WIDTH, TEX_HEIGHT, &tm, NUM_SAMPLES),
			GL_DEPTH_ATTACHMENT) ||
		!fbRightEyeDraw.addTexture(
			createTexture2DMulti(
				TexInfo(GL_TEXTURE_2D_MULTISAMPLE, { int(TEX_WIDTH), int(TEX_HEIGHT) }, 0, GL_RGB, GL_RGB16F, GL_FLOAT), &tm, NUM_SAMPLES),
			GL_COLOR_ATTACHMENT1))
	{
		std::cout << "FBO creation failed" << endl;
	}

	Framebuffer aoRenderLeft = createNewFramebuffer(TEX_WIDTH, TEX_HEIGHT);
	Framebuffer aoRenderRight = createNewFramebuffer(TEX_WIDTH, TEX_HEIGHT);
	aoRenderLeft.addTexture(createTexture2DMulti(
		TEX_WIDTH, TEX_HEIGHT, &tm, NUM_SAMPLES),
		GL_COLOR_ATTACHMENT0);
	aoRenderRight.addTexture(createTexture2DMulti(
		TEX_WIDTH, TEX_HEIGHT, &tm, NUM_SAMPLES),
		GL_COLOR_ATTACHMENT0);


	//TEST ^^



	if (!fbLeftEyeRead.addTexture(
		createTexture2D(TEX_WIDTH, TEX_HEIGHT, &tm), GL_COLOR_ATTACHMENT0)) {
		std::cout << "FBO creation failed" << endl;
	}
	else if (!fbRightEyeRead.addTexture(
		createTexture2D(TEX_WIDTH, TEX_HEIGHT, &tm), GL_COLOR_ATTACHMENT0)) {
		std::cout << "FBO creation failed" << endl;
	}

	Viewport leftEyeView(window_width / 2, window_height);
	Viewport rightEyeView(window_width / 2, window_height, window_width / 2);

	//Parse tracked devices
	int headsetIndex = 0;
	vector<VRController> controllers;
	vr::TrackedDevicePose_t poses[vr::k_unMaxTrackedDeviceCount];

	for (int i = 0; i < vr::k_unMaxTrackedDeviceCount; i++) {
		vr::TrackedDeviceClass deviceClass = vrDisplay->GetTrackedDeviceClass(i);

		switch (deviceClass) {
		case vr::TrackedDeviceClass_HMD:
			headsetIndex = i;
			break;
		case vr::TrackedDeviceClass_Controller:
			controllers.push_back(VRController(i, vrDisplay, poses[i], &tm));
			break;
		}
	}

	VRCameraController vrCam(&poses[headsetIndex], vrDisplay);

	//Dragon
	auto dragonGeom = shared_ptr<ElementGeometry>(objToElementGeometry("models/dragon.obj"));
	Drawable dragon(
		dragonGeom,
		make_shared<ColorMat>(vec3(0.75f, 0.1f, 0.3f)));
	dragon.addMaterial(new ShadedMat(0.2f, 0.5f, 0.3f, 10.f));

	dragon.setPosition(vec3(1.f, 0, 0));
	dragon.setOrientation(angleAxis(-PI*0.5f, vec3(0.f, 1.f, 0.f)));

	auto sphereGeom = shared_ptr<ElementGeometry>(objToElementGeometry("models/icosphere.obj"));
	Drawable sphere(
		sphereGeom,
		make_shared<ColorMat>(vec3(0.1, 0.3f, 0.8f)));
	sphere.addMaterial(new ShadedMat(0.2f, 0.5f, 0.3f, 10.f));

	sphere.setPosition(vec3(1.f, 0, 0));

	//Squares for left and right views
	//TEST vv
	Drawable leftSquare(
		new SimpleTexGeometry(points, coords, 6, GL_TRIANGLES),
		new TextureMat(fbLeftEyeDraw.getTexture(GL_COLOR_ATTACHMENT0), TextureMat::POSITION));

	Drawable rightSquare(
		new SimpleTexGeometry(points, coords, 6, GL_TRIANGLES),
		new TextureMat(fbRightEyeDraw.getTexture(GL_COLOR_ATTACHMENT0), TextureMat::POSITION));

	leftSquare.addMaterial(new TextureMat(fbLeftEyeDraw.getTexture(GL_COLOR_ATTACHMENT1), TextureMat::NORMAL));
	rightSquare.addMaterial(new TextureMat(fbRightEyeDraw.getTexture(GL_COLOR_ATTACHMENT1), TextureMat::NORMAL));

	AOShader aoShader;
	PosNormalShader pnShader;

	Drawable leftSquareTest(
		new SimpleTexGeometry(points, coords2, 6, GL_TRIANGLES),
		new TextureMat(fbLeftEyeRead.getTexture(GL_COLOR_ATTACHMENT0)));

	Drawable rightSquareTest(
		new SimpleTexGeometry(points, coords2, 6, GL_TRIANGLES),
		new TextureMat(fbRightEyeRead.getTexture(GL_COLOR_ATTACHMENT0)));

	//TEST ^^

	SimpleTexShader texShader;
	SimpleShader shader;
	BlinnPhongShader bpShader;
	BlinnPhongShader bpTexShader({ { GL_FRAGMENT_SHADER, "#define USING_TEXTURE\n" }
	});

	TrackballCamera savedCam = cam;

	vec3 lightPos(-100.f, 100.f, 100.f);

	fbLeftEyeDraw.use();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	bpShader.draw(cam, lightPos, dragon);

	fbWindow.use();

	vector<Drawable> drawables;
	//	loadWavefront("untrackedmodels/OrganodronCity/", "OrganodronCity", &drawables, &tm);
	//	loadWavefront("untrackedmodels/SciFiCenter/CenterCity/", "scificity", &drawables, &tm);
	//	loadWavefront("untrackedmodels/lstudio/", "lsystem.obj", &drawables, &tm);
	//	loadWavefront("untrackedmodels/", "riccoSurface_take2", &drawables, &tm);

	auto objGeometry = shared_ptr<ElementGeometry>(objToElementGeometry("untrackedmodels/riccoSurface_take3.obj"));
	drawables.push_back(Drawable(objGeometry, make_shared<ShadedMat>(0.3, 0.4, 0.4, 10.f)));
	drawables[0].addMaterial(new ColorMat(vec3(1, 1, 1)));


//	drawables.push_back(dragon);

	for (int i = 0; i < drawables.size(); i++) {
		drawables[i].setPosition(vec3(0, 0, -1.f));
		drawables[i].setScale(vec3(10.0));
	}

	vector<vec3> controllerPositions(controllers.size());

	quat perFrameRot = angleAxis(3.14159f / 90.f, vec3(0, 1, 0));

	//Velocity
	vec3 linearVelocity(0.f);
	quat angularVelocity = quat();

	while (!glfwWindowShouldClose(window)) {
		if (reloadShaders) {
			aoShader.createProgram();
			reloadShaders = false;
		}


		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//Get pose
		vr::VRCompositor()->WaitGetPoses(poses, vr::k_unMaxTrackedDeviceCount,
			NULL, 0);

		//Update camera
		vrCam.update();
		vrCam.setProjection(vrDisplay, 0.2f, 400.f);

		//Update controllers
		vector<int> triggersPressed;
		for (int i = 0; i < controllers.size(); i++) {
			vr::VRControllerState_t state;
			vr::TrackedDevicePose_t pose;
			if (!vrDisplay->GetControllerStateWithPose(
				vr::VRCompositor()->GetTrackingSpace(),
				controllers[i].index,
				&state,
				sizeof(vr::VRControllerState_t),
				&pose))
			{
				printf("Error reading controller state\n");
			}

			controllers[i].updatePose(poses[controllers[i].index]);
			controllers[i].updateState(state);
			if (controllers[i].axes[VRController::TRIGGER_AXIS].x > 0.5f)
				triggersPressed.push_back(i);
		}

		vec3 positionTransform(0.f);
		quat orientationTransform;

		bool updatePositions = true;

		switch (triggersPressed.size()) {
		case 1:
		{
			vec3 lastPos = controllerPositions[triggersPressed[0]];
			positionTransform = controllers[triggersPressed[0]].getPos() - lastPos;
			linearVelocity = positionTransform;
			break;
		}
		case 2:
		{
			vec3 axisA = normalize(controllerPositions[triggersPressed[0]]
				- controllerPositions[triggersPressed[1]]);
			vec3 axisB = normalize(controllers[triggersPressed[0]].getPos()
				- controllers[triggersPressed[1]].getPos());
			vec3 rotAxis = cross(axisA, axisB);
			if (length(rotAxis) > 0.0001) {
				float angle = asin(length(rotAxis));
				orientationTransform = angleAxis(angle, normalize(rotAxis));
				angularVelocity = orientationTransform;
			}
			else
				updatePositions = false;
			break;

		}
		default:
			for (int i = 0; i < drawables.size(); i++) {
				quat orientation = drawables[i].getOrientationQuat();
				//				drawables[i].setOrientation(normalize(angularVelocity*orientation));
				drawables[i].setPosition(drawables[i].getPos() + linearVelocity);
			}

			//angularVelocity = slerp(angularVelocity, quat(), 0.002f);
			linearVelocity *= 0.97f;
		}

		if (updatePositions) {
			for (int i = 0; i < controllerPositions.size(); i++) {
				controllerPositions[i] = controllers[i].getPos();
			}
		}


		//Update model
		for (int i = 0; i < drawables.size(); i++) {
			quat orientation = drawables[i].getOrientationQuat();
			drawables[i].setOrientation(normalize(orientationTransform*orientation));
			drawables[i].setPosition(drawables[i].getPos() + positionTransform);

		}

		glEnable(GL_MULTISAMPLE);		//TEST
		glClearColor(0.f, 0.f, 0.f, 1.f);

		dragon.setPosition(0.5f*vrCam.leftEye.getPosition()
			+ 0.5f*vrCam.rightEye.getPosition()
			+ vec3(0, 2, 0));


		//TEST vv AMBIENT OCCLUSION
		//Draw left eye
		fbLeftEyeDraw.use();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		for (int i = 0; i < controllers.size(); i++)
			pnShader.draw(vrCam.leftEye, lightPos, controllers[i]);

		for (int i = 0; i < drawables.size(); i++) {
			pnShader.draw(vrCam.leftEye, lightPos, drawables[i]);
		}

		//Ambient occlusion
		aoRenderLeft.use();
		aoShader.draw(cam, lightPos, leftSquare);

		//Draw right eye
		fbRightEyeDraw.use();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		for (int i = 0; i < controllers.size(); i++)
			pnShader.draw(vrCam.rightEye, lightPos, controllers[i]);
		for (int i = 0; i < drawables.size(); i++) {
			pnShader.draw(vrCam.rightEye, lightPos, drawables[i]);
		}

		//Ambient occlusion
		aoRenderRight.use();
		aoShader.draw(cam, lightPos, rightSquare);

		//TEST ^^ 

		glDisable(GL_MULTISAMPLE);

		blit(aoRenderLeft, fbLeftEyeRead);
		blit(aoRenderRight, fbRightEyeRead);

		//Draw window
		fbWindow.use();
		leftEyeView.use();
		glClearColor(1.0, 1.0, 1.0, 1.0);
		texShader.draw(cam, leftSquareTest);

		rightEyeView.use();
		glClearColor(1.0, 1.0, 1.0, 1.0);
		texShader.draw(cam, rightSquareTest);

		//Draw headset
		if (vrDisplay) {
			vr::Texture_t leftTexture = {
				(void*)(uintptr_t)GLuint(fbLeftEyeRead.getTexture(GL_COLOR_ATTACHMENT0).getID()),
				vr::TextureType_OpenGL, vr::ColorSpace_Gamma };
			vr::Texture_t rightTexture = {
				(void*)(uintptr_t)GLuint(fbRightEyeRead.getTexture(GL_COLOR_ATTACHMENT0).getID()),
				vr::TextureType_OpenGL, vr::ColorSpace_Gamma };

			vr::VRCompositor()->Submit(vr::Eye_Left, &leftTexture);
			vr::VRCompositor()->Submit(vr::Eye_Right, &rightTexture);
		}

		checkGLErrors("Buffer overflow?");

		glfwSwapBuffers(window);
		glfwPollEvents();
	}


	glfwTerminate();
	vr::VR_Shutdown();
	*/
}

void initGlad() {
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cout << "GLAD initialization failed" << std::endl;
	}
}

vr::IVRSystem *initVR() {

	vr::EVRInitError error = vr::VRInitError_None;
	vr::IVRSystem *vrDisplay = vr::VR_Init(&error, vr::VRApplication_Scene);

	if (error != vr::VRInitError_None)
	{
		vrDisplay = nullptr;
		std::cout << "[Error]" << VR_GetVRInitErrorAsSymbol(error) << std::endl;
	}
	else if (!vr::VR_GetGenericInterface(vr::IVRRenderModels_Version, &error)) {
		printf("Failed to obtain render models\n");
		vrDisplay = nullptr;
	}
	else if (!vr::VRCompositor()) {
		printf("Compositor failed\n");
		vrDisplay = nullptr;
	}

	return vrDisplay;
}

void WindowManager::initGL() {
	glClearColor(1.f, 1.f, 1.f, 1.f);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	glViewport(0, 0, window_width, window_height);
}

GLFWwindow *createWindow(int width, int height, std::string name)
{
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow *window = glfwCreateWindow(
		width, height, name.c_str(), nullptr, nullptr);

	if (window == NULL) {
		glfwTerminate();
		return nullptr;
	}
	else {
		glfwMakeContextCurrent(window);
		return window;
	}
}

