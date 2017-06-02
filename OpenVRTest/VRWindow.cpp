#include "VRWindow.h"

#include <iostream>

using namespace glm;
using namespace std;

#include "Drawable.h"
#include "SimpleGeometry.h"
#include "SimpleShader.h"
#include "ColorMat.h"
#include "TrackballCamera.h"
#include "SimpleTexManager.h"
#include "simpleTexShader.h"
#include "TextureCreation.h"
#include "TextureMat.h"
#include "MeshInfoLoader.h"
#include "ShadedMat.h"
#include "TorranceSparrowShader.h"
#include "Framebuffer.h"


#include <glm/gtc/matrix_transform.hpp>

const float PI = 3.14159265358979323846;

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

WindowManager::WindowManager() :
window_width(800), window_height(800)
{
	glfwInit();
	window = createWindow(window_width, window_height, 
		"You really should rename this");
	initGlad();
	vrDisplay = initVR();

	glfwSwapInterval(0);

	initGL();
}

WindowManager::WindowManager(int width, int height, std::string name, glm::vec4 color) :
	window_width(width), window_height(height) 
{
	glfwInit();
	window = createWindow(window_width, window_height, name);
	initGlad();
	vrDisplay = initVR();

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

//Temporary testing
void WindowManager::mainLoop() {

	glfwSetCursorPosCallback(window, cursorPositionCallback);

	vec3 points [6] = {
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
//	glfwSetWindowSize(window, TEX_WIDTH * 2, TEX_HEIGHT);


	Framebuffer fbLeftEyeDraw = createNewFramebuffer(TEX_WIDTH, TEX_HEIGHT);
	Framebuffer fbRightEyeDraw = createNewFramebuffer(TEX_WIDTH, TEX_HEIGHT);
	Framebuffer fbLeftEyeRead = createNewFramebuffer(TEX_WIDTH, TEX_HEIGHT);
	Framebuffer fbRightEyeRead = createNewFramebuffer(TEX_WIDTH, TEX_HEIGHT);


	const int NUM_SAMPLES = 4;

	if (!fbLeftEyeDraw.addTexture(
//		createTexture2DMulti(TEX_WIDTH, TEX_HEIGHT, &tm, NUM_SAMPLES),
		createTexture2D(TEX_WIDTH, TEX_HEIGHT, &tm),
		GL_COLOR_ATTACHMENT0) ||
		!fbLeftEyeDraw.addTexture(
//		createDepthTextureMulti(TEX_WIDTH, TEX_HEIGHT, &tm, NUM_SAMPLES), GL_DEPTH_ATTACHMENT)) 
		createDepthTexture(TEX_WIDTH, TEX_HEIGHT, &tm), GL_DEPTH_ATTACHMENT))
	{
		std::cout << "FBO creation failed" << endl;
	}
	if (!fbRightEyeDraw.addTexture(
//		createTexture2DMulti(TEX_WIDTH, TEX_HEIGHT, &tm, NUM_SAMPLES),
		createTexture2D(TEX_WIDTH, TEX_HEIGHT, &tm),
		GL_COLOR_ATTACHMENT0) ||
		!fbRightEyeDraw.addTexture(
//		createDepthTextureMulti(TEX_WIDTH, TEX_HEIGHT, &tm, NUM_SAMPLES), GL_DEPTH_ATTACHMENT))
		createDepthTexture(TEX_WIDTH, TEX_HEIGHT, &tm), GL_DEPTH_ATTACHMENT))
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
	Viewport rightEyeView(window_width / 2, window_height, window_width/2);

	vr::TrackedDevicePose_t poses[vr::k_unMaxTrackedDeviceCount];
	VRCameraController vrCam (&poses[0], vrDisplay);

	//Dragon
	ElementGeometry dragonGeom = objToElementGeometry("models/dragon.obj");
	Drawable dragon(
		new ColorMat(vec3(0.75f, 0.1f, 0.3f)),
		&dragonGeom);
	dragon.addMaterial(new ShadedMat(0.2f, 0.5f, 0.3f, 10.f));

	dragon.setPosition(vec3(1.f, 0, 0));
	dragon.setOrientation(angleAxis(-PI*0.5f, vec3(0.f, 1.f, 0.f)));

	ElementGeometry sphereGeom = objToElementGeometry("models/icosphere.obj");
	Drawable sphere(
		new ColorMat(vec3(0.1, 0.3f, 0.8f)),
		&sphereGeom);
	sphere.addMaterial(new ShadedMat(0.2f, 0.5f, 0.3f, 10.f));

	sphere.setPosition(vec3(1.f, 0, 0));

	//Squares for left and right views
	Drawable leftSquare(
		new TextureMat(fbLeftEyeRead.getTexture(GL_COLOR_ATTACHMENT0)),
		new SimpleTexGeometry(points, coords, 6, GL_TRIANGLES));

	Drawable rightSquare(
		new TextureMat(fbRightEyeRead.getTexture(GL_COLOR_ATTACHMENT0)),
		new SimpleTexGeometry(points, coords, 6, GL_TRIANGLES));

	SimpleTexShader texShader;
	SimpleShader shader;
	TorranceSparrowShader tsShader;

	TrackballCamera savedCam = cam;

	vec3 lightPos(-10.f, 10.f, 1.f);

	fbLeftEyeDraw.use();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	tsShader.draw(cam, lightPos, dragon);

	fbWindow.use();

	while (!glfwWindowShouldClose(window)) {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//Get pose
		vr::VRCompositor()->WaitGetPoses(poses, 1, NULL, 0);

		//Update camera
		vrCam.update();
		vrCam.setProjection(vrDisplay);

	/*	static vec3 pos(0.f);
		vec3 newPos = vrCam.leftEye.getPosition();
		if (glm::distance(newPos, pos) > 0.01f) {
			pos = newPos;
			vec4 dir = inverse(vrCam.leftEye.getCameraMatrix())*vec4(0, 0, 1, 0);
			printf("Pos = (%f, %f, %f) Dir = (%f, %f, %f)\n", 
				pos.x, pos.y, pos.z, dir.x, dir.y, dir.z);

			printMat4(vrCam.leftEye.getCameraMatrix());
			printMat4(vrCam.leftEye.getProjectionMatrix());
		}*/
	
		lightPos = 0.5f*vrCam.leftEye.getPosition() + 
			0.5f*vrCam.rightEye.getPosition();

		//dragon.setPosition(lightPos+vec3(dir.x, dir.y, dir.z));

		glEnable(GL_MULTISAMPLE);

		//Draw left eye
		fbLeftEyeDraw.use();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		tsShader.draw(vrCam.leftEye, lightPos, dragon);

		//Draw right eye
		fbRightEyeDraw.use();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		tsShader.draw(vrCam.rightEye, lightPos, dragon);

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

		if (vrDisplay) {
			vr::Texture_t leftTexture = { 
				(void*)(uintptr_t)fbLeftEyeRead.getTexture(GL_COLOR_ATTACHMENT0).getID(), 
				vr::TextureType_OpenGL, vr::ColorSpace_Gamma };
			vr::Texture_t rightTexture = {
				(void*)(uintptr_t)fbRightEyeRead.getTexture(GL_COLOR_ATTACHMENT0).getID(),
				vr::TextureType_OpenGL, vr::ColorSpace_Gamma };

			vr::VRCompositor()->Submit(vr::Eye_Left, &leftTexture);
			vr::VRCompositor()->Submit(vr::Eye_Right, &rightTexture);
		}

		glfwSwapBuffers(window);
		glfwPollEvents();
	}


	delete leftSquare.getMaterial(TextureMat::id);
	delete leftSquare.getGeometryPtr();

	delete dragon.getMaterial(ColorMat::id);
	delete dragon.getMaterial(ShadedMat::id);
	
	delete sphere.getMaterial(ColorMat::id);
	delete sphere.getMaterial(ShadedMat::id);

	fbLeftEyeDraw.deleteFramebuffer();
	fbLeftEyeDraw.deleteTextures();
	fbRightEyeDraw.deleteFramebuffer();
	fbRightEyeDraw.deleteTextures();

	glfwTerminate();
	vr::VR_Shutdown();
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
	else if (!vr::VR_GetGenericInterface(vr::IVRRenderModels_Version, &error)){
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
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
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

