#include "Cube.h"
#include "Plane.h"
#include "Cave.h"
#include "shader.h"

#include <iostream>
#include <memory>
#include <exception>
#include <algorithm>
#include <Windows.h>
#include <math.h>

//#include <limits>

#define _CRT_SECURE_NO_WARNINGS //
#define __STDC_FORMAT_MACROS 1

#define FAIL(X) throw std::runtime_error(X)

///////////////////////////////////////////////////////////////////////////////
//
// GLM is a C++ math library meant to mirror the syntax of GLSL 
//

#include <glm/glm.hpp>
#include <glm/gtc/noise.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>


// Import the most commonly used types into the default namespace
using glm::ivec3;
using glm::ivec2;
using glm::uvec2;
using glm::mat3;
using glm::mat4;
using glm::vec2;
using glm::vec3;
using glm::vec4;
using glm::quat;

using namespace std;

///////////////////////////////////////////////////////////////////////////////
//
// GLEW gives cross platform access to OpenGL 3.x+ functionality.  
//

#include <GL/glew.h>

//glm::vec3 hand; // hand position

/////// Custom variables
bool cube_size_up = false; // set to true with LThumbStick to right
bool cube_size_down = false; // set to true with LThumbStick to left
bool cube_size_reset = false; // set to true with LThumbStick pressed in

double iod = 0.0;
double original_iod = 0.0;
bool iod_up = false; // true when RThumbStick to right
bool iod_down = false; // true when RThumbStick to left
bool iod_reset = false; // true when RThumbStick pressed in

// Button X controls
bool x1 = true; // show entire scene (cubes and sky box in stereo)
bool x2 = false; // show just the sky box in stereo
bool x3 = false; // show just the sky box in mono
bool x4 = false; // show room instead of bear skybox

// Button A controls
bool a1 = true; // 3D stereo
bool a2 = false; // mono (the same image rendered on both eyes)
bool a3 = false; // left eye only (right eye black)
bool a4 = false; // right eye only (left eye black)
bool a5 = false; // inverted stereo (left eye image rendered to right eye and vice versa)

// Button B controls
//bool b1 = true; // regular head tracking (both position and orientation)
//bool b2 = false; // orientation only (position frozen to what it just was before the mode was selected)

bool isPressed = false; // true if any button is pressed

// HMD transformation matrices
glm::mat4 headPos_left_curr = glm::mat4(1.0f);
glm::mat4 headPos_right_curr = glm::mat4(1.0f);
glm::mat4 headPos_left_prev = glm::mat4(1.0f); // use this matrix to track head position each frame
glm::mat4 headPos_right_prev = glm::mat4(1.0f);

glm::vec3 hand = glm::vec3(1.0f);

//Cube * controller;
Cave * cave;
Plane * plane; // temp use only
//////////////////////

bool checkFramebufferStatus(GLenum target = GL_FRAMEBUFFER) {
	GLuint status = glCheckFramebufferStatus(target);
	switch (status) {
	case GL_FRAMEBUFFER_COMPLETE:
		return true;
		break;

	case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
		std::cerr << "framebuffer incomplete attachment" << std::endl;
		break;

	case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
		std::cerr << "framebuffer missing attachment" << std::endl;
		break;

	case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
		std::cerr << "framebuffer incomplete draw buffer" << std::endl;
		break;

	case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
		std::cerr << "framebuffer incomplete read buffer" << std::endl;
		break;

	case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
		std::cerr << "framebuffer incomplete multisample" << std::endl;
		break;

	case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS:
		std::cerr << "framebuffer incomplete layer targets" << std::endl;
		break;

	case GL_FRAMEBUFFER_UNSUPPORTED:
		std::cerr << "framebuffer unsupported internal format or image" << std::endl;
		break;

	default:
		std::cerr << "other framebuffer error" << std::endl;
		break;
	}

	return false;
}

bool checkGlError() {
	GLenum error = glGetError();
	if (!error) {
		return false;
	}
	else {
		switch (error) {
		case GL_INVALID_ENUM:
			std::cerr << ": An unacceptable value is specified for an enumerated argument.The offending command is ignored and has no other side effect than to set the error flag.";
			break;
		case GL_INVALID_VALUE:
			std::cerr << ": A numeric argument is out of range.The offending command is ignored and has no other side effect than to set the error flag";
			break;
		case GL_INVALID_OPERATION:
			std::cerr << ": The specified operation is not allowed in the current state.The offending command is ignored and has no other side effect than to set the error flag..";
			break;
		case GL_INVALID_FRAMEBUFFER_OPERATION:
			std::cerr << ": The framebuffer object is not complete.The offending command is ignored and has no other side effect than to set the error flag.";
			break;
		case GL_OUT_OF_MEMORY:
			std::cerr << ": There is not enough memory left to execute the command.The state of the GL is undefined, except for the state of the error flags, after this error is recorded.";
			break;
		case GL_STACK_UNDERFLOW:
			std::cerr << ": An attempt has been made to perform an operation that would cause an internal stack to underflow.";
			break;
		case GL_STACK_OVERFLOW:
			std::cerr << ": An attempt has been made to perform an operation that would cause an internal stack to overflow.";
			break;
		}
		return true;
	}
}

void glDebugCallbackHandler(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *msg, GLvoid* data) {
	OutputDebugStringA(msg);
	std::cout << "debug call: " << msg << std::endl;
}

//////////////////////////////////////////////////////////////////////
//
// GLFW provides cross platform window creation
//

#include <GLFW/glfw3.h>

namespace glfw {
	inline GLFWwindow * createWindow(const uvec2 & size, const ivec2 & position = ivec2(INT_MIN)) {
		GLFWwindow * window = glfwCreateWindow(size.x, size.y, "glfw", nullptr, nullptr);
		if (!window) {
			FAIL("Unable to create rendering window");
		}
		if ((position.x > INT_MIN) && (position.y > INT_MIN)) {
			glfwSetWindowPos(window, position.x, position.y);
		}
		return window;
	}
}

// A class to encapsulate using GLFW to handle input and render a scene
class GlfwApp {

protected:
	uvec2 windowSize;
	ivec2 windowPosition;
	GLFWwindow * window{ nullptr };
	unsigned int frame{ 0 };

public:
	GlfwApp() {
		// Initialize the GLFW system for creating and positioning windows
		if (!glfwInit()) {
			FAIL("Failed to initialize GLFW");
		}
		glfwSetErrorCallback(ErrorCallback);
	}

	virtual ~GlfwApp() {
		if (nullptr != window) {
			glfwDestroyWindow(window);
		}
		glfwTerminate();
	}

	virtual int run() {
		preCreate();

		window = createRenderingTarget(windowSize, windowPosition);

		if (!window) {
			std::cout << "Unable to create OpenGL window" << std::endl;
			return -1;
		}

		postCreate();

		initGl();

		while (!glfwWindowShouldClose(window)) {
			++frame;
			glfwPollEvents();
			update();
			draw();
			finishFrame();
		}

		shutdownGl();

		return 0;
	}


protected:
	virtual GLFWwindow * createRenderingTarget(uvec2 & size, ivec2 & pos) = 0;

	virtual void draw() = 0;

	void preCreate() {
		glfwWindowHint(GLFW_DEPTH_BITS, 16);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
		glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);
	}


	void postCreate() {
		glfwSetWindowUserPointer(window, this);
		glfwSetKeyCallback(window, KeyCallback);
		glfwSetMouseButtonCallback(window, MouseButtonCallback);
		glfwMakeContextCurrent(window);

		// Initialize the OpenGL bindings
		// For some reason we have to set this experminetal flag to properly
		// init GLEW if we use a core context.
		glewExperimental = GL_TRUE;
		if (0 != glewInit()) {
			FAIL("Failed to initialize GLEW");
		}
		glGetError();

		if (GLEW_KHR_debug) {
			GLint v;
			glGetIntegerv(GL_CONTEXT_FLAGS, &v);
			if (v & GL_CONTEXT_FLAG_DEBUG_BIT) {
				//glDebugMessageCallback(glDebugCallbackHandler, this);
			}
		}
	}

	virtual void initGl() {
	}

	virtual void shutdownGl() {
	}

	virtual void finishFrame() {
		glfwSwapBuffers(window);
	}

	virtual void destroyWindow() {
		glfwSetKeyCallback(window, nullptr);
		glfwSetMouseButtonCallback(window, nullptr);
		glfwDestroyWindow(window);
	}

	virtual void onKey(int key, int scancode, int action, int mods) {
		if (GLFW_PRESS != action) {
			return;
		}

		switch (key) {
		case GLFW_KEY_ESCAPE:
			glfwSetWindowShouldClose(window, 1);
			return;
		}
	}

	virtual void update() {}

	virtual void onMouseButton(int button, int action, int mods) {}

protected:
	virtual void viewport(const ivec2 & pos, const uvec2 & size) {
		glViewport(pos.x, pos.y, size.x, size.y);
	}

private:

	static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
		GlfwApp * instance = (GlfwApp *)glfwGetWindowUserPointer(window);
		instance->onKey(key, scancode, action, mods);
	}

	static void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
		GlfwApp * instance = (GlfwApp *)glfwGetWindowUserPointer(window);
		instance->onMouseButton(button, action, mods);
	}

	static void ErrorCallback(int error, const char* description) {
		FAIL(description);
	}
};

//////////////////////////////////////////////////////////////////////
//
// The Oculus VR C API provides access to information about the HMD
//

#include <OVR_CAPI.h>
#include <OVR_CAPI_GL.h>

namespace ovr {

	// Convenience method for looping over each eye with a lambda
	template <typename Function>
	inline void for_each_eye(Function function) {
		for (ovrEyeType eye = ovrEyeType::ovrEye_Left;
			eye < ovrEyeType::ovrEye_Count;
			eye = static_cast<ovrEyeType>(eye + 1)) {
			function(eye);
		}
	}

	inline mat4 toGlm(const ovrMatrix4f & om) {
		return glm::transpose(glm::make_mat4(&om.M[0][0]));
	}

	inline mat4 toGlm(const ovrFovPort & fovport, float nearPlane = 0.01f, float farPlane = 10000.0f) {
		return toGlm(ovrMatrix4f_Projection(fovport, nearPlane, farPlane, true));
	}

	inline vec3 toGlm(const ovrVector3f & ov) {
		return glm::make_vec3(&ov.x);
	}

	inline vec2 toGlm(const ovrVector2f & ov) {
		return glm::make_vec2(&ov.x);
	}

	inline uvec2 toGlm(const ovrSizei & ov) {
		return uvec2(ov.w, ov.h);
	}

	inline quat toGlm(const ovrQuatf & oq) {
		return glm::make_quat(&oq.x);
	}

	inline mat4 toGlm(const ovrPosef & op) {
		mat4 orientation = glm::mat4_cast(toGlm(op.Orientation));
		mat4 translation = glm::translate(mat4(), ovr::toGlm(op.Position));
		return translation * orientation;
	}

	inline ovrMatrix4f fromGlm(const mat4 & m) {
		ovrMatrix4f result;
		mat4 transposed(glm::transpose(m));
		memcpy(result.M, &(transposed[0][0]), sizeof(float) * 16);
		return result;
	}

	inline ovrVector3f fromGlm(const vec3 & v) {
		ovrVector3f result;
		result.x = v.x;
		result.y = v.y;
		result.z = v.z;
		return result;
	}

	inline ovrVector2f fromGlm(const vec2 & v) {
		ovrVector2f result;
		result.x = v.x;
		result.y = v.y;
		return result;
	}

	inline ovrSizei fromGlm(const uvec2 & v) {
		ovrSizei result;
		result.w = v.x;
		result.h = v.y;
		return result;
	}

	inline ovrQuatf fromGlm(const quat & q) {
		ovrQuatf result;
		result.x = q.x;
		result.y = q.y;
		result.z = q.z;
		result.w = q.w;
		return result;
	}
}

class RiftManagerApp {
protected:
	ovrSession _session;
	ovrHmdDesc _hmdDesc;
	ovrGraphicsLuid _luid;

public:
	RiftManagerApp() {
		if (!OVR_SUCCESS(ovr_Create(&_session, &_luid))) {
			FAIL("Unable to create HMD session");
		}

		_hmdDesc = ovr_GetHmdDesc(_session);
	}

	~RiftManagerApp() {
		ovr_Destroy(_session);
		_session = nullptr;
	}
};

class RiftApp : public GlfwApp, public RiftManagerApp {
public:
	GLuint _fbo{ 0 };

private:
	// new framebuffer
	//GLuint _fbo_2{ 0 };

	/*GLuint _fbo{ 0 };*/
	GLuint _depthBuffer{ 0 };
	ovrTextureSwapChain _eyeTexture;

	GLuint _mirrorFbo{ 0 };
	ovrMirrorTexture _mirrorTexture;

	ovrEyeRenderDesc _eyeRenderDescs[2];

	mat4 _eyeProjections[2];

	ovrLayerEyeFov _sceneLayer;
	ovrViewScaleDesc _viewScaleDesc;

	uvec2 _renderTargetSize;
	uvec2 _mirrorSize;

public:

	RiftApp() {
		using namespace ovr;
		_viewScaleDesc.HmdSpaceToWorldScaleInMeters = 1.0f;

		memset(&_sceneLayer, 0, sizeof(ovrLayerEyeFov));
		_sceneLayer.Header.Type = ovrLayerType_EyeFov;
		_sceneLayer.Header.Flags = ovrLayerFlag_TextureOriginAtBottomLeft;

		ovr::for_each_eye([&](ovrEyeType eye) {
			ovrEyeRenderDesc& erd = _eyeRenderDescs[eye] = ovr_GetRenderDesc(_session, eye, _hmdDesc.DefaultEyeFov[eye]);
			ovrMatrix4f ovrPerspectiveProjection =
				ovrMatrix4f_Projection(erd.Fov, 0.01f, 1000.0f, ovrProjection_ClipRangeOpenGL);
			_eyeProjections[eye] = ovr::toGlm(ovrPerspectiveProjection);

			// cse190: adjust the eye separation here - need to use 3D vector from central point on Rift for each eye
			_viewScaleDesc.HmdToEyePose[eye] = erd.HmdToEyePose; 

			// get IOD see slides
		    /*iod = abs(_viewScaleDesc.HmdToEyePose[0].Position.x - _viewScaleDesc.HmdToEyePose[1].Position.x);
			original_iod = abs(_viewScaleDesc.HmdToEyePose[0].Position.x - _viewScaleDesc.HmdToEyePose[1].Position.x);*/
			
			ovrFovPort & fov = _sceneLayer.Fov[eye] = _eyeRenderDescs[eye].Fov;
			auto eyeSize = ovr_GetFovTextureSize(_session, eye, fov, 1.0f);
			_sceneLayer.Viewport[eye].Size = eyeSize;
			_sceneLayer.Viewport[eye].Pos = { (int)_renderTargetSize.x, 0 };

			_renderTargetSize.y = std::max(_renderTargetSize.y, (uint32_t)eyeSize.h);
			_renderTargetSize.x += eyeSize.w;
		});
		// Make the on screen window 1/4 the resolution of the render target
		_mirrorSize = _renderTargetSize;
		_mirrorSize /= 2; // was 4 before

	}

protected:
	GLFWwindow * createRenderingTarget(uvec2 & outSize, ivec2 & outPosition) override {
		return glfw::createWindow(_mirrorSize);
	}

	void initGl() override {
		GlfwApp::initGl();

		// Disable the v-sync for buffer swap
		glfwSwapInterval(0);

		ovrTextureSwapChainDesc desc = {};
		desc.Type = ovrTexture_2D;
		desc.ArraySize = 1;
		desc.Width = _renderTargetSize.x;
		desc.Height = _renderTargetSize.y;
		desc.MipLevels = 1;
		desc.Format = OVR_FORMAT_R8G8B8A8_UNORM_SRGB;
		desc.SampleCount = 1;
		desc.StaticImage = ovrFalse;
		ovrResult result = ovr_CreateTextureSwapChainGL(_session, &desc, &_eyeTexture);
		_sceneLayer.ColorTexture[0] = _eyeTexture;
		if (!OVR_SUCCESS(result)) {
			FAIL("Failed to create swap textures");
		}

		int length = 0;
		result = ovr_GetTextureSwapChainLength(_session, _eyeTexture, &length);
		if (!OVR_SUCCESS(result) || !length) {
			FAIL("Unable to count swap chain textures");
		}
		for (int i = 0; i < length; ++i) {
			GLuint chainTexId;
			ovr_GetTextureSwapChainBufferGL(_session, _eyeTexture, i, &chainTexId);
			glBindTexture(GL_TEXTURE_2D, chainTexId);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		}
		glBindTexture(GL_TEXTURE_2D, 0);

		// Set up the framebuffer object
		glGenFramebuffers(1, &_fbo);
		/*glGenFramebuffers(1, &_fbo_2);*/

		glGenRenderbuffers(1, &_depthBuffer);
		
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, _fbo);
		/*glBindFramebuffer(GL_DRAW_FRAMEBUFFER, _fbo_2);*/

		glBindRenderbuffer(GL_RENDERBUFFER, _depthBuffer);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, _renderTargetSize.x, _renderTargetSize.y);
		glBindRenderbuffer(GL_RENDERBUFFER, 0);
		glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, _depthBuffer);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

		ovrMirrorTextureDesc mirrorDesc;
		memset(&mirrorDesc, 0, sizeof(mirrorDesc));
		mirrorDesc.Format = OVR_FORMAT_R8G8B8A8_UNORM_SRGB;
		mirrorDesc.Width = _mirrorSize.x;
		mirrorDesc.Height = _mirrorSize.y;
		if (!OVR_SUCCESS(ovr_CreateMirrorTextureGL(_session, &mirrorDesc, &_mirrorTexture))) {
			FAIL("Could not create mirror texture");
		}
		glGenFramebuffers(1, &_mirrorFbo);

	}

	void update() final override
	{
		ovrInputState inputState;
		if (OVR_SUCCESS(ovr_GetInputState(_session, ovrControllerType_Touch, &inputState)))
		{
			// reset booleans
			cube_size_up = false;
			cube_size_down = false;
			cube_size_reset = false;

			// Logic to vary interocular distance
			if (inputState.Thumbstick[ovrHand_Right].x > 0) {
				//cout << "iod up" << endl;
				iod_up = true;
			}
			else if (inputState.Thumbstick[ovrHand_Right].x < 0) {
				//cout << "iod down" << endl;
				iod_down = true;
			}
			else if (inputState.Buttons & ovrButton_RThumb) {
				//cout << "iod reset" << endl;
				iod_reset = true;
			}

			////////////////////////
			// Logic to resize cubes
			if (inputState.Thumbstick[ovrHand_Left].x < 0) {
				//cout << "left thumbstick to the left" << endl;
				cube_size_down = true;
				//cube_size_up = false;
			}
			else if (inputState.Thumbstick[ovrHand_Left].x > 0) {
				//cout << "left thumbstick to the right" << endl; 
				cube_size_up = true;
				//cube_size_down = false;
			}
			else if (inputState.Buttons && ovrButton_LThumb) {
				//cout << "left thubstick pressed in" << endl;
				cube_size_reset = true;
			}


			if (!inputState.Buttons) {
				isPressed = false;
			}

			///////////////////////////////
			// Logic to cycle between five modes with the 'A' button
			if ((inputState.Buttons & ovrButton_A) && !isPressed) {
				//cout << "Button A pressed" << endl;
				isPressed = true;

				if (a1) {
					a1 = false;
					a2 = true;
					//cout << "monoscopic mode (left eye image rendered on both eyes)" << endl;
				}
				else if (a2) {
					a2 = false;
					a3 = true;
					//cout << "only rendering to left eye" << endl;
				}
				else if (a3) {
					a3 = false;
					a4 = true;
					//cout << "only rendering to right eye" << endl;
				}
				else if (a4) {
					a4 = false;
					a5 = true;
					//cout << "inverted stereo mode" << endl;
				}
				else if (a5) {
					a1 = true;
					a5 = false;
					//cout << "back to default mode" << endl;
				}
			}

			///////////////////////////////
			// Logic to cycle between five modes with the 'X' button
			else if ((inputState.Buttons & ovrButton_X) && !isPressed) {

				isPressed = true;

				if (x1) {
					x1 = false;
					x2 = true;
					//cout << "showing just the sky box in stereo" << endl;
				}
				else if (x2) {
					x2 = false;
					x3 = true;
					//cout << "showing just the sky box in mono" << endl;
				}
				else if (x3) {
					x3 = false;
					x4 = true;
					//cout << "showing my room" << endl;
				}
				else if (x4) {
					x4 = false;
					x1 = true;
					//cout << "showing the entire scene" << endl;
				}
			}

			///////////////////////////////
			// Logic to cycle between four head tracking modes with the 'B' button
			else if ((inputState.Buttons & ovrButton_B) && !isPressed) {

				isPressed = true;

				if (b1) {
					b1 = false;
					b2 = true;
					//cout << "orientation only (position frozen to what it just was before the mode was selected)" << endl;
				}
				else if (b2) {
					b2 = false;
					b1 = true;
				}
			}
		}
		/////////////
		// change iod

		// reset booleans
		iod_up = false;
		iod_down = false;
		iod_reset = false;
	}

	void onKey(int key, int scancode, int action, int mods) override {
		if (GLFW_PRESS == action) switch (key) {
		case GLFW_KEY_R:
			ovr_RecenterTrackingOrigin(_session);
			return;
		}

		GlfwApp::onKey(key, scancode, action, mods);
	}

	void draw() final override {

		// Query Touch controllers. Query their parameters:
		double displayMidpointSeconds = ovr_GetPredictedDisplayTime(_session, 0);
		ovrTrackingState trackState = ovr_GetTrackingState(_session, displayMidpointSeconds, ovrTrue);

		// Process controller status. Useful to know if controller is being used at all, and if the cameras can see it. 
		// Bits reported:
		// Bit 1: ovrStatus_OrientationTracked  = Orientation is currently tracked (connected and in use)
		// Bit 2: ovrStatus_PositionTracked     = Position is currently tracked (false if out of range)
		unsigned int handStatus[2];
		handStatus[0] = trackState.HandStatusFlags[0];
		handStatus[1] = trackState.HandStatusFlags[1];
		// Display status for debug purposes:
		//cerr << "handStatus[left]  = " << handStatus[ovrHand_Left] << endl;
		//cerr << "handStatus[right] = " << handStatus[ovrHand_Right] << endl;

		// Process controller position and orientation:
		ovrPosef handPoses[2];  // These are position and orientation in meters in room coordinates, relative to tracking origin. Right-handed cartesian coordinates.
								// ovrQuatf     Orientation;
								// ovrVector3f  Position;
		handPoses[0] = trackState.HandPoses[0].ThePose;
		handPoses[1] = trackState.HandPoses[1].ThePose;
		ovrVector3f handPosition[2];
		handPosition[0] = handPoses[0].Position;
		handPosition[1] = handPoses[1].Position;
		
		// update controller position
		hand.x = handPosition[ovrHand_Left].x;
		hand.y = handPosition[ovrHand_Left].y;
		hand.z = handPosition[ovrHand_Left].z;
		
		// Display positions for debug purposes:
		//cerr << "left hand position  = " << handPosition[ovrHand_Left].x << ", " << handPosition[ovrHand_Left].y << ", " << handPosition[ovrHand_Left].z << endl;
		//cerr << "right hand position = " << handPosition[ovrHand_Right].x << ", " << handPosition[ovrHand_Right].y << ", " << handPosition[ovrHand_Right].z << endl;
		/////////////////////////////////////////

		ovrPosef eyePoses[2];
		ovr_GetEyePoses(_session, frame, true, _viewScaleDesc.HmdToEyePose, eyePoses, &_sceneLayer.SensorSampleTime);

		///////////////////////////////
		///// head positions this frame
		headPos_left_curr = ovr::toGlm(eyePoses[ovrEye_Left]);
		headPos_right_curr = ovr::toGlm(eyePoses[ovrEye_Right]);
		// here used to be "B" button controls

		//headPos_left_curr[3] = headPos_left_prev[3];
		//headPos_right_curr[3] = headPos_right_prev[3];

		int curIndex;
		ovr_GetTextureSwapChainCurrentIndex(_session, _eyeTexture, &curIndex);
		GLuint curTexId;
		ovr_GetTextureSwapChainBufferGL(_session, _eyeTexture, curIndex, &curTexId);
		
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, _fbo);
		//glBindFramebuffer(GL_DRAW_FRAMEBUFFER, _fbo_2);

		glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, curTexId, 0);
		
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		/////// LOOK OVER HERE
		ovr::for_each_eye([&](ovrEyeType eye) {
			const auto& vp = _sceneLayer.Viewport[eye];
			glViewport(vp.Pos.x, vp.Pos.y, vp.Size.w, vp.Size.h);
			_sceneLayer.RenderPose[eye] = eyePoses[eye];
			
			if (a1) {
				// normal stereo rendering; call renderScene() twice one time for each eye
				if (eye == ovrEye_Left) {
					renderScene(_eyeProjections[ovrEye_Left], headPos_left_curr, true);
				}
				else {
					renderScene(_eyeProjections[ovrEye_Right], headPos_right_curr, false);
				}			
			}

			else if (a2) {
				// render one eye's view to both eyes = monoscopic view
				renderScene(_eyeProjections[eye], headPos_left_curr, true);
			}
			else if (a3) {
				// render to only left eye
				if (eye == ovrEye_Left) {
					renderScene(_eyeProjections[ovrEye_Left], headPos_left_curr, true);
				}
				
			}
			else if (a4) {
				// render to only right eye
				if (eye == ovrEye_Right) {
					renderScene(_eyeProjections[ovrEye_Right], headPos_right_curr, false);
				}
			}
			else if (a5) {
				// render left eye to right eye and vice versa - inverted stereo
				if (eye == ovrEye_Left) renderScene(_eyeProjections[ovrEye_Right], headPos_right_curr, false);
				if (eye == ovrEye_Right) renderScene(_eyeProjections[ovrEye_Left], headPos_left_curr, true);
			}

		});
		// put renderscene outside

		glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, 0, 0);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
		ovr_CommitTextureSwapChain(_session, _eyeTexture);
		ovrLayerHeader* headerList = &_sceneLayer.Header;
		ovr_SubmitFrame(_session, frame, &_viewScaleDesc, &headerList, 1);

		GLuint mirrorTextureId;
		ovr_GetMirrorTextureBufferGL(_session, _mirrorTexture, &mirrorTextureId);
		glBindFramebuffer(GL_READ_FRAMEBUFFER, _mirrorFbo);
		glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mirrorTextureId, 0);
		glBlitFramebuffer(0, 0, _mirrorSize.x, _mirrorSize.y, 0, _mirrorSize.y, _mirrorSize.x, 0, GL_COLOR_BUFFER_BIT, GL_NEAREST);
		glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);


		/////////// store head positions from the previous frame
		headPos_left_prev = headPos_left_curr;
		headPos_right_prev = headPos_right_curr;
	}

	/*virtual void renderScene(const glm::mat4 & projection, const glm::mat4 & headPose) = 0;*/
	virtual void renderScene(const glm::mat4 & projection, const glm::mat4 & headPose, bool isLeft) = 0;

	// new renderCave() method; need at least projection and view as parameter
};


// An example application that renders a simple cube
class ExampleApp : public RiftApp {
	//std::shared_ptr<ColorCubeScene> cubeScene;

public:
	ExampleApp() { }


protected:
	void initGl() override {
		RiftApp::initGl();
		glClearColor(0.2f, 0.3f, 0.8f, 1.0f); // change background color to light blue

		glEnable(GL_DEPTH_TEST);
		ovr_RecenterTrackingOrigin(_session);
		//cubeScene = std::shared_ptr<ColorCubeScene>(new ColorCubeScene());

		cave = new Cave();
	}

	void shutdownGl() override {
		//cubeScene.reset();
	}


	// newly defined function
	void renderScene(const glm::mat4 & projection, const glm::mat4 & headPose, bool isLeft) {

		//cout << "isLeft: " << isLeft << endl;
		
		cave->renderCave(projection, glm::inverse(headPose), isLeft, _fbo);
		
		cave->renderController(projection, glm::inverse(headPose), hand);
		
		//cave->render(projection, glm::inverse(headPose), isLeft);
		
		//cubeScene->render(projection, glm::inverse(headPose), isLeft);
	}
};
 
// Execute our example class
int main(int argc, char** argv)
{
	int result = -1;
	try {
		if (!OVR_SUCCESS(ovr_Initialize(nullptr))) {
			FAIL("Failed to initialize the Oculus SDK");
		}
		result = ExampleApp().run();
	}
	catch (std::exception & error) {
		OutputDebugStringA(error.what());
		std::cerr << error.what() << std::endl;
	}
	ovr_Shutdown();
	return result;
}