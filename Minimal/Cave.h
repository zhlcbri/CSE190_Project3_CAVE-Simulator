#ifndef Cave_h
#define Cave_h

#include <iostream>
#include <stdio.h>
#include <vector>
#include <string> //

#include <Windows.h> //

#define GLFW_INCLUDE_GLEXT
#ifdef __APPLE__
#define GLFW_INCLUDE_GLCOREARB
#else
#include <GL/glew.h>
#endif
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/noise.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

#include <OVR_CAPI.h>
#include <OVR_CAPI_GL.h>

#include <GL/glew.h>

#define STB_IMAGE_IMPLEMENTATION
#define _USE_MATH_DEFINES
#include <math.h>

#include "stb_image.h"
#include "shader.h"

using namespace std;
using namespace glm;

// HMD position of the current and previous frame
mat4 headPos_curr = mat4(1.0f);
mat4 headPos_prev = mat4(1.0f);

// position vector and matrix of left controller
vec3 hand = vec3(1.0f);
mat4 handMat = mat4(1.0f);;

// HMD (eye) transformation matrices
mat4 headPos_left_curr = mat4(1.0f);
mat4 headPos_right_curr = mat4(1.0f);
mat4 headPos_left_prev = mat4(1.0f); 
mat4 headPos_right_prev = mat4(1.0f);

// freeze viewpoint when set to true
bool freeze_view = false; 

// enter wireframe debug mode when set to true
bool debug_mode = false;

// switch head position to left controller position when set to true
// head-in-hand mode
bool head_in_hand = false;

// average human eye distance: 65 millimeters
float IOD = 6.5f;

// cube movements
bool cube_left, cube_right, cube_up, cube_down = false; 
bool cube_forward, cube_backward, cube_pos_reset = false;
// cube scaling
bool cube_size_up, cube_size_down, cube_size_reset = false; 

// variables used to calculate projection matrices for each CAVE screen
vec3 PA = vec3(-1.0f, -1.0f, 0.0f);
vec3 PB = vec3(1.0f, -1.0f, 0.0f);
vec3 PC = vec3(-1.0f, 1.0f, 0.0f);

Cube * skybox_room;
GLuint cube_shader;

class Cave {
private:

public:
	Cube * cube_1;
	Cube * skybox_left;
	Cube * skybox_right;
	//Cube * skybox_room;
	Cube * controller;
	Plane * plane_1;

	//GLuint cube_shader;
	GLuint plane_shader;
	GLuint FBO, textureColorbuffer, rbo;

	GLuint tempTex; // temp

	GLsizei WIDTH = 1280;
	GLsizei HEIGHT = 720;

	vector<string> cube_faces = {
		"cube_pattern.ppm",
		"cube_pattern.ppm",
		"cube_pattern.ppm",
		"cube_pattern.ppm",
		"cube_pattern.ppm",
		"cube_pattern.ppm"
	};

	vector<string> skybox_faces_room = {
		// 2 are flipped horizontally
		// 3 are flipped vertically
		"skybox_room/px_2.ppm",
		"skybox_room/nx_2.ppm",
		"skybox_room/py_3.ppm",
		"skybox_room/ny_3.ppm",
		"skybox_room/nz_2.ppm",
		"skybox_room/pz_2.ppm",
	};

	vector<string> skybox_faces_left = {
		"skybox_leftEye/nx.ppm",
		"skybox_leftEye/px.ppm",
		"skybox_leftEye/py_2.ppm", // rotated 
		"skybox_leftEye/ny_2.ppm", // rotated
		"skybox_leftEye/nz.ppm",
		"skybox_leftEye/pz.ppm",
	};

	vector<string> skybox_faces_right = {
		"skybox_rightEye/nx.ppm",
		"skybox_rightEye/px.ppm",
		"skybox_rightEye/py_2.ppm", // rotated 
		"skybox_rightEye/ny_2.ppm", // rotated
		"skybox_rightEye/nz.ppm",
		"skybox_rightEye/pz.ppm",
	};

	const char * tex_temp = "cube_pattern.ppm"; // temp texture to test quad

	const char * CUBE_VERT_PATH = "shader_cube.vert";
	const char * CUBE_FRAG_PATH = "shader_cube.frag";

	const char * PLANE_VERT_PATH = "shader_plane.vert";
	const char * PLANE_FRAG_PATH = "shader_plane.frag";

	vec3 cubePos = vec3(0.0f, 0.0f, -4.0f);
	mat4 cubeScaleMat = scale(mat4(1.0f), vec3(0.3f, 0.3f, 0.3f)); // only mat used to scale cube

	mat4 quadScaleMat = scale(mat4(1.0f), vec3(8.0f, 8.0f, 8.0f)); // only mat used to scale quad

	// Constructor
	// ---------------------------------------------------
	Cave() {
		skybox_left = new Cube(skybox_faces_left, true, true, false);
		skybox_right = new Cube(skybox_faces_right, true, false, false);
		skybox_room = new Cube(skybox_faces_room, true, false, true);
		cube_1 = new Cube(cube_faces, false, false, false); // calibration cube
		controller = new Cube(cube_faces, false, false, false); // controller
		plane_1 = new Plane();

		cube_shader = LoadShaders(CUBE_VERT_PATH, CUBE_FRAG_PATH);
		plane_shader = LoadShaders(PLANE_VERT_PATH, PLANE_FRAG_PATH);

		tempTex = loadTexture(tex_temp);

		// shader configuration - maybe move to Plane::draw()
		glUseProgram(plane_shader);
		glUniform1i(glGetUniformLocation(plane_shader, "screenTexture"), 0);

		// framebuffer configuration
		glGenFramebuffers(1, &FBO);
		glBindFramebuffer(GL_FRAMEBUFFER, FBO);

		// create a color attachment texture
		glGenTextures(1, &textureColorbuffer);
		glBindTexture(GL_TEXTURE_2D, textureColorbuffer);		

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, /*GL_TEXTURE_WIDTH*/WIDTH, /*GL_TEXTURE_HEIGHT*/HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL); // TEXTURE_WIDTH?
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		// to render whole screen to a texture call glViewport() before rendering to framebuffer with the new dimensions of texture
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColorbuffer, 0);

		// create a renderbuffer object for depth and stencil attachment (we won't be sampling these)
		glGenRenderbuffers(1, &rbo);
		glBindRenderbuffer(GL_RENDERBUFFER, rbo);
		// use a single renderbuffer object for both a depth AND stencil buffer.
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, /*GL_TEXTURE_WIDTH*/WIDTH, /*GL_TEXTURE_HEIGHT*/HEIGHT);
		/*glBindRenderbuffer(GL_RENDERBUFFER, 0);*/
		
		// now actually attach it
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo); 
									
		// now that we actually created the framebuffer and added all attachments we want to check if it is actually complete now
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << endl;
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	};

	// Destructor
	// ---------------------------------------------------
	~Cave() {
		delete(skybox_left);
		delete(skybox_right);
		delete(cube_1);
		delete(controller);
		delete(plane_1);
		glDeleteProgram(cube_shader);
		glDeleteProgram(plane_shader);
		glDeleteBuffers(1, &FBO);
		glDeleteBuffers(1, &textureColorbuffer);
		glDeleteBuffers(1, &rbo);
	};
	
	void resetCubes() {
		cubeScaleMat = glm::scale(glm::mat4(1.0f), glm::vec3(0.3f, 0.3f, 0.3f));
	};

	void scaleCubes(float val) {
		cubeScaleMat = glm::scale(glm::mat4(1.0f), glm::vec3(val)) * cubeScaleMat; // which on first
	};

	void moveCubes(float delta_x, float delta_y, float delta_z) {
		cubePos = glm::vec3(cubePos.x + delta_x, cubePos.y + delta_y, cubePos.z + delta_z);
	}

	// Rendering cubes
	// ---------------------------------------------------
	void renderCubes(const mat4 & projection, const mat4 & modelview, GLuint uProjection) {

		// change cubeScaleMat according to booleans
		if (cube_size_up) {
			if (cubeScaleMat[0][0] < 0.5f && cubeScaleMat[1][1] < 0.5f && cubeScaleMat[2][2] < 0.5f) {
				scaleCubes(1.01f);
			}
		}
		else if (cube_size_down) {
			if (cubeScaleMat[0][0] > 0.1f && cubeScaleMat[1][1] > 0.1f && cubeScaleMat[2][2] > 0.1f) {
				scaleCubes(0.99f);
			}
		}
		else if (cube_size_reset) {
			resetCubes();
		}

		if (cube_up) {
			//moveCubes(0.0f, 0.01f, 0.0f);
		}


		if (cube_left && cubePos.x > -0.5f) {
			//moveCubes(-0.01f, 0.0f, 0.0f);
		}
		else if (cube_right && cubePos.x < 0.5f) {
			//moveCubes(0.01f, 0.0f, 0.0f);
		}
		else if (cube_forward && cubePos.y > -0.5f) {
			//cout << "cube forward" << endl; //
			//moveCubes(0.0f, 0.01f, 0.0f);
		}
		else if (cube_backward && cubePos.y > -0.5f) {
			//cout << "cube backward" << endl; //
			//moveCubes(0.0f, -0.01f, 0.0f);
		}
		else if (cube_pos_reset) {
			//cubePos = vec3(0.0f, 0.0f, -4.0f);
		}

		glm::mat4 posMat = glm::translate(glm::mat4(1.0f), cubePos);
		glm::mat4 posMat_in = glm::translate(glm::mat4(1.0f), -cubePos);
		glm::mat4 M = posMat * cubeScaleMat * posMat_in;

		// draw closer cube
		glUniformMatrix4fv(uProjection, 1, GL_FALSE, &M[0][0]);
		cube_1->draw(cube_shader, projection, modelview);
	};

	// Rendering a smaller cube at left controller position
	// ---------------------------------------------------
	void renderController(const mat4 & projection, const mat4 & modelview, vec3 handPos) {
		
		// specify positions
		mat4 posMat = translate(mat4(1.0f), handPos);
		mat4 posMat_in = translate(mat4(1.0f), -handPos);
		mat4 scaleMat = scale(mat4(1.0f), vec3(0.02f, 0.02f, 0.02f));
		mat4 M = posMat * scaleMat;

		// draw cube at controller position
		glUseProgram(cube_shader);
		GLuint uModel = glGetUniformLocation(cube_shader, "model");
		glUniformMatrix4fv(uModel, 1, GL_FALSE, &M[0][0]);
		controller->draw(cube_shader, projection, modelview);
	};

	// Rendering custom skybox to wrap the entire cave
	// ---------------------------------------------------
	void renderRoom(const mat4 & projection, const mat4 & modelview) {

		glUseProgram(cube_shader);
		GLuint uModel = glGetUniformLocation(cube_shader, "model");
		mat4 scaleMat = scale(mat4(1.0f), vec3(100.0f, 100.0f, 100.0f));
		glUniformMatrix4fv(uModel, 1, GL_FALSE, &scaleMat[0][0]);

		isRoom = true;
		skybox_room->draw(cube_shader, projection, modelview);
		isRoom = false;
	}

	// Rendering main scene (skybox and calibration cube)
	// ---------------------------------------------------
	void render(const mat4 & projection, const mat4 & modelview, bool isLeftEye) {

		// current head matrix
		headPos_curr = modelview;

		// freeze head orientation
		if (freeze_view) {
			headPos_curr[0] = headPos_prev[0];
			headPos_curr[1] = headPos_prev[1];
			headPos_curr[2] = headPos_prev[2];
		}	

		// temp: head-in-hand mode
		//headPos_curr[3] = vec4(hand, 1.0f);

		// shader configuration
		glUseProgram(cube_shader);
		GLuint uModel = glGetUniformLocation(cube_shader, "model");

		// render skybox
		mat4 scaleMat = scale(glm::mat4(1.0f), glm::vec3(100.0f, 100.0f, 100.0f));
		glUniformMatrix4fv(uModel, 1, GL_FALSE, &scaleMat[0][0]);

		// render different texture images for left and right eye to create stereo effect
		if (isLeftEye) {
			skybox_left->draw(cube_shader, projection, /*modelview)*/headPos_curr);
		}
		else {		
			skybox_right->draw(cube_shader, projection, /*modelview)*/headPos_curr);
		}

		renderCubes(projection, /*modelview)*/headPos_curr, uModel);

		// store head matrix from the last frame
		headPos_prev = headPos_curr;
	};

	// calculate projection matrix for off-screen rendering
	// ---------------------------------------------------
	mat4 getProjectionMatrix(mat4 model, bool isLeftEye) {
		vec3 p_a = (vec3)(model * vec4(PA.x, PA.y, PA.z, 1.0f));
		vec3 p_b = (vec3)(model * vec4(PB.x, PB.y, PB.z, 1.0f));
		vec3 p_c = (vec3)(model * vec4(PC.x, PC.y, PC.z, 1.0f));
		vec3 p_e = vec3(1.0f);

		// different eye positions for each eye
		if (isLeftEye) {
			p_e = (vec3)headPos_left_curr[3];
		}
		else {
			p_e = (vec3)headPos_right_curr[3];
		}

		// vectors from eye to corners
		vec3 v_a = p_a - p_e;
		vec3 v_b = p_b - p_e;
		vec3 v_c = p_c - p_e;

		// screen-local axes which give us a basis for describing points relative to the screen
		vec3 v_r = (p_b - p_a) / distance(p_b, p_a);
		vec3 v_u = (p_c - p_a) / distance(p_c, p_a);
		vec3 v_n = cross(v_r, v_u) / length(cross(v_r, v_u));

		// near and far clipping plane (n and f units away)
		float n = 0.01f;
		float f = 1000.0f;

		// distance from eye to plane
		float d = -dot(v_n, v_a);

		// calculate frustum parameters
		float l = dot(v_r, v_a) * (n / d);
		float r = dot(v_r, v_b) * (n / d);
		float b = dot(v_u, v_a) * (n / d);
		float t = dot(v_u, v_c) * (n / d);

		// projection matrix for CAVE screens
		mat4 P = frustum(l, r, b, t, n, f);
		
		// inverse of screen coordinate system
		// transform objects lying in screen plane to XY plane
		mat4 M_T = mat4(1.0f);
		
		M_T[0] = vec4(v_r.x, v_u.x, v_n.x, 0.0f);
		M_T[1] = vec4(v_r.y, v_u.y, v_n.y, 0.0f);
		M_T[2] = vec4(v_r.z, v_u.z, v_n.z, 0.0f);

		// view point offset
		mat4 T = mat4(1.0f);
		T[3] = vec4(-p_e.x, -p_e.y, -p_e.z, 1.0f);

		// actual projection that we want to return
		mat4 P_prime = P * M_T * T;

		return P_prime;
	}

	// Rendering screens
	// ---------------------------------------------------
	void renderQuads(const mat4 & projection, const mat4 & modelview, GLuint uModel, bool isLeftEye) {
		// specify position
		vec3 pos_1 = vec3(0.0f, 0.0f, -8.0f);
		// translation matrix (T)
		mat4 posMat = translate(mat4(1.0f), pos_1);
		// rotation matrix (R)
		mat4 rotateMat = rotate(mat4(1.0f), (float)(45 * M_PI) / 180, vec3(0.0f, 1.0f, 0.0f));
		// model matrix (M = T*S*R)
		mat4 M = posMat * quadScaleMat * rotateMat;

		// draw 1st quad
		glUniformMatrix4fv(uModel, 1, GL_FALSE, &M[0][0]);
		plane_1->draw(plane_shader, textureColorbuffer, projection, modelview);

		//////////////////////////////////

		// draw 2nd quad
		rotateMat = glm::rotate(mat4(1.0f), -(float)(45 * M_PI) / 180, vec3(0.0f, 1.0f, 0.0f));
		M = posMat * quadScaleMat * rotateMat;
		glUniformMatrix4fv(uModel, 1, GL_FALSE, &M[0][0]);
		plane_1->draw(plane_shader, textureColorbuffer, projection, modelview);

		//////////////////////////////////

		// draw 3rd quad as floor
		rotateMat = glm::rotate(mat4(1.0f), (float)(45 * M_PI) / 180, vec3(0.0f, 0.0f, 1.0f));
		rotateMat = glm::rotate(mat4(1.0f), -(float)(90 * M_PI) / 180, vec3(1.0f, 0.0f, 0.0f)) * rotateMat;
		M = posMat * quadScaleMat * rotateMat;
		glUniformMatrix4fv(uModel, 1, GL_FALSE, &M[0][0]);
		plane_1->draw(plane_shader, textureColorbuffer, projection, modelview);

	};

	// Rendering scene on screen
	// ---------------------------------------------------
	void renderCave(const mat4 & projection, const mat4 & modelview, bool isLeftEye, GLuint old_FBO, ovrLayerEyeFov sceneLayer) {
		
		glBindFramebuffer(GL_FRAMEBUFFER, FBO); // bind to new framebuffer and draw scene as we normally would to color texture 
		glEnable(GL_DEPTH_TEST); // enable depth testing
		
		glClearColor(0.4f, 0.5f, 0.3f, 1.0f); // clear the framebuffer's content
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // not using the stencil buffer now

		// setup the viewport for each eye before rendering the quads
		if (!isLeftEye) {
			glViewport(sceneLayer.Viewport[0].Pos.x, sceneLayer.Viewport[0].Pos.y, sceneLayer.Viewport[0].Size.w, sceneLayer.Viewport[0].Size.h);
		}
		else {
			glViewport(sceneLayer.Viewport[1].Pos.x, sceneLayer.Viewport[1].Pos.y, sceneLayer.Viewport[1].Size.w, sceneLayer.Viewport[1].Size.h);
		}

		// draw scene (skybox and cube) to framebuffer with the actual projection
		mat4 P_prime = mat4(1.0f);

		if (head_in_hand) {
			P_prime = getProjectionMatrix(handMat, isLeftEye);
			render(P_prime, handMat, isLeftEye);
		}
		else {
			P_prime = getProjectionMatrix(modelview, isLeftEye);
			render(P_prime, modelview, isLeftEye);
		}

		render(P_prime/*projection*/, modelview, isLeftEye);

		glBindFramebuffer(GL_FRAMEBUFFER, old_FBO); // bind back to default framebuffer
		glDisable(GL_DEPTH_TEST); // disable depth test so screen-space quad isn't discarded due to depth test.

		// draw CAVE screens with the attached framebuffer color texture
		glUseProgram(plane_shader); 
		GLuint uModel = glGetUniformLocation(plane_shader, "model");
		renderQuads(projection, modelview, uModel, isLeftEye);

		glClearColor(0.4f, 0.5f, 0.3f, 1.0f); // clear all relevant buffers
	};

	// utility function for loading a 2D texture from file
	// ---------------------------------------------------
	unsigned int loadTexture(char const * path)
	{
		unsigned int textureID;
		glGenTextures(1, &textureID);

		int width, height, nrComponents;
		unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);
		if (data)
		{
			GLenum format;
			if (nrComponents == 1)
				format = GL_RED;
			else if (nrComponents == 3)
				format = GL_RGB;
			else if (nrComponents == 4)
				format = GL_RGBA;

			glBindTexture(GL_TEXTURE_2D, textureID);
			glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
			glGenerateMipmap(GL_TEXTURE_2D);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			stbi_image_free(data);
		}
		else
		{
			std::cout << "Texture failed to load at path: " << path << std::endl;
			stbi_image_free(data);
		}

		return textureID;
	}

};

#endif