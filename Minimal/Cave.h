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
#include "Triangle.h"

using namespace std;
using namespace glm;

GLsizei WIDTH = 1000;
GLsizei HEIGHT = 1000;

// HMD position of the current and previous frame
mat4 headPos_curr = mat4(1.0f);
mat4 headPos_prev = mat4(1.0f);

// projection and modelview matrix of the current and previous frame
mat4 projection_curr = mat4(1.0f);
mat4 projection_prev = mat4(1.0f);
mat4 modelview_curr = mat4(1.0f);
mat4 modelview_prev = mat4(1.0f);

// position vector and matrix of left controller
vec3 hand = vec3(1.0f);
mat4 handMat = mat4(1.0f);;

// translation matrices for 3 quad screens
mat4 quadModel_1 = mat4(1.0f);
mat4 quadModel_2 = mat4(1.0f);
mat4 quadModel_3 = mat4(1.0f);

// HMD (eye) transformation matrices
mat4 headPos_left_curr = mat4(1.0f);
mat4 headPos_right_curr = mat4(1.0f);
mat4 headPos_left_prev = mat4(1.0f); 
mat4 headPos_right_prev = mat4(1.0f);

// one projector failure on right eye when set to true
bool screen_fail = false;

// freeze viewpoint when set to true
bool freeze_view = false; 

// enter wireframe debug mode when set to true
bool debug_mode = false;

// switch head position to left controller position when set to true
bool head_in_hand = false;

// average human eye distance: 65 millimeters
float IOD = 6.5f;

// cube movements and scaling
bool cube_left, cube_right, cube_up, cube_down = false; 
bool cube_forward, cube_backward, cube_pos_reset = false;
bool cube_size_up, cube_size_down, cube_size_reset = false; 

// variables used to calculate projection matrices for each CAVE screen
vec3 PA = vec3(-1.0f, -1.0f, 0.0f);
vec3 PB = vec3(1.0f, -1.0f, 0.0f);
vec3 PC = vec3(-1.0f, 1.0f, 0.0f);

// objects
Cube * skybox_room;
GLuint cube_shader;
GLuint plane_shader;
GLuint pyramid_shader;

Triangle * triangle_1;
Triangle * triangle_2;
Triangle * triangle_3;

class Cave {
private:

public:
	Cube * cube_1;
	Cube * skybox_left;
	Cube * skybox_right;
	//Cube * skybox_room;
	Cube * controller;
	
	Plane * plane_1;
	Plane * plane_2;
	Plane * plane_3;
	//Triangle * triangle_1;

	//GLuint cube_shader;
	//GLuint plane_shader;
	GLuint FBO, textureColorbuffer, RBO;
	GLuint FBO_2, FBO_3;
	GLuint textureColorbuffer_2, textureColorbuffer_3;
	GLuint RBO_2, RBO_3;

	GLuint tempTex; // temp; delete later

	/*GLsizei WIDTH = 1000;
	GLsizei HEIGHT = 1000;*/

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

	const char * PYRAMID_VERT_PATH = "shader_pyramid.vert";
	const char * PYRAMID_FRAG_PATH = "shader_pyramid.frag";

	vec3 cubePos = vec3(0.0f, 0.0f, -5.0f);
	mat4 cubeScaleMat = scale(mat4(1.0f), vec3(0.3f, 0.3f, 0.3f)); // only mat used to scale cube

	mat4 quadScaleMat = scale(mat4(1.0f), /*vec3(2.0f, 2.0f, 2.0f)*/vec3(8.0f, 8.0f, 8.0f)); // only mat used to scale quad

	// Constructor
	// ---------------------------------------------------
	Cave() {
		skybox_left = new Cube(skybox_faces_left, true, true, false);
		skybox_right = new Cube(skybox_faces_right, true, false, false);
		skybox_room = new Cube(skybox_faces_room, true, false, true);
		
		cube_1 = new Cube(cube_faces, false, false, false); // calibration cube
		controller = new Cube(cube_faces, false, false, false); // controller
		
		/*plane_1 = new Plane();
		plane_2 = new Plane();
		plane_3 = new Plane();*/

		plane_1 = new Plane(FBO, textureColorbuffer, RBO);
		plane_2 = new Plane(FBO_2, textureColorbuffer_2, RBO_2);
		plane_3 = new Plane(FBO_3, textureColorbuffer_3, RBO_3);

		triangle_1 = new Triangle(vertices);
		triangle_2 = new Triangle(vertices);
		triangle_3 = new Triangle(vertices);
		
		cube_shader = LoadShaders(CUBE_VERT_PATH, CUBE_FRAG_PATH);
		plane_shader = LoadShaders(PLANE_VERT_PATH, PLANE_FRAG_PATH);
		pyramid_shader = LoadShaders(PYRAMID_VERT_PATH, PYRAMID_FRAG_PATH);

		tempTex = loadTexture(tex_temp); // delete

		////////////////
		// model matrix for 1st quad
		vec3 pos_1 = vec3(0.0f, /*0.0f*/-3.0f, -8.0f);
		mat4 posMat = translate(mat4(1.0f), pos_1);
		mat4 rotateMat = rotate(mat4(1.0f), 45 * pi<float>() / 180, vec3(0.0f, 1.0f, 0.0f));
		quadModel_1 = posMat * quadScaleMat * rotateMat; // M = T*S*R
		//quadModel_1 = posMat * mat4(1.0f);

		// model matrix for 2nd quad
		rotateMat = glm::rotate(mat4(1.0f), -45 * pi<float>() / 180, vec3(0.0f, 1.0f, 0.0f));
		quadModel_2 = posMat * quadScaleMat * rotateMat;

		// model matrix for 3rd quad
		/*rotateMat = glm::rotate(mat4(1.0f), (float)(45 * M_PI) / 180, vec3(0.0f, 0.0f, 1.0f));
		rotateMat = glm::rotate(mat4(1.0f), -(float)(90 * M_PI) / 180, vec3(1.0f, 0.0f, 0.0f)) * rotateMat;*/

		/*rotateMat = glm::rotate(mat4(1.0f), -90 * pi<float>() / 180, vec3(1.0f, 0.0f, 0.0f));
		rotateMat = glm::rotate(mat4(1.0f), 45 * pi<float>() / 180, vec3(0.0f, 1.0f, 0.0f)) * rotateMat;*/

		mat4 T_b = translate(mat4(1.0f), vec3(0.0f, -3.0f, -8.0f));
		rotateMat = /*T_b * */rotate(glm::mat4(1.0f), -1.0f / 2.0f * pi<float>(), vec3(1.0f, 0.0f, 0.0f)) 
			* rotate(glm::mat4(1.0f), -1.0f / 4.0f * pi<float>(), vec3(0.0f, 0.0f, 1.0f));

		quadModel_3 =  T_b /** posMat*/ * quadScaleMat * rotateMat;
		

		// framebuffer configuration for 1st quad
		glGenFramebuffers(1, &FBO);
		glBindFramebuffer(GL_FRAMEBUFFER, FBO);

		// create color attachment textures
		glGenTextures(1, &textureColorbuffer);
		glBindTexture(GL_TEXTURE_2D, textureColorbuffer);		
		
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, /*GL_TEXTURE_WIDTH*/WIDTH, /*GL_TEXTURE_HEIGHT*/HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL); // TEXTURE_WIDTH?
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		// to render whole screen to a texture call glViewport() before rendering to framebuffer with the new dimensions of texture
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColorbuffer, 0);
		
		// create a renderbuffer object for depth and stencil attachment (we won't be sampling these)
		glGenRenderbuffers(1, &RBO);
		glBindRenderbuffer(GL_RENDERBUFFER, RBO);
		// use a single renderbuffer object for both a depth AND stencil buffer.
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, /*GL_TEXTURE_WIDTH*/WIDTH, /*GL_TEXTURE_HEIGHT*/HEIGHT);

		// now actually attach it
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, RBO);

		// now that we actually created the framebuffer and added all attachments we want to check if it is actually complete now
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << endl;
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		//////////////
		glGenFramebuffers(1, &FBO_2);
		glBindFramebuffer(GL_FRAMEBUFFER, FBO_2);

		glGenTextures(1, &textureColorbuffer_2);
		glBindTexture(GL_TEXTURE_2D, textureColorbuffer_2);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, WIDTH, HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColorbuffer_2, 0);
		
		glGenRenderbuffers(1, &RBO_2);
		glBindRenderbuffer(GL_RENDERBUFFER, RBO_2);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, WIDTH, HEIGHT);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, RBO_2);

		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << endl;
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		///////////////////////
		glGenFramebuffers(1, &FBO_3);
		glBindFramebuffer(GL_FRAMEBUFFER, FBO_3);

		glGenTextures(1, &textureColorbuffer_3);
		glBindTexture(GL_TEXTURE_2D, textureColorbuffer_3);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, WIDTH, HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL); 
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColorbuffer_3, 0);

		glGenRenderbuffers(1, &RBO_3);
		glBindRenderbuffer(GL_RENDERBUFFER, RBO_3);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, WIDTH, HEIGHT);

		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, RBO_3);
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << endl;
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		/////////////////////////////


		
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
		glDeleteBuffers(1, &FBO_2);
		glDeleteBuffers(1, &FBO_3);
		glDeleteBuffers(1, &textureColorbuffer);
		glDeleteBuffers(1, &textureColorbuffer_2);
		glDeleteBuffers(1, &textureColorbuffer_3);
		glDeleteBuffers(1, &RBO);
		glDeleteBuffers(1, &RBO_2);
		glDeleteBuffers(1, &RBO_3);
	};

	void moveCubes(float delta_x, float delta_y, float delta_z) {
		cubePos = glm::vec3(cubePos.x + delta_x, cubePos.y + delta_y, cubePos.z + delta_z);
	}

	void resetCubes() {
		cubeScaleMat = glm::scale(glm::mat4(1.0f), glm::vec3(0.3f, 0.3f, 0.3f));
	};

	void repositionCubes() {
		cubePos = vec3(0.0f, 0.0f, -4.0f);
	}

	void scaleCubes(float val) {
		cubeScaleMat = cubeScaleMat * glm::scale(glm::mat4(1.0f), glm::vec3(val)); // which one first
	};

	// Rendering cubes
	// ---------------------------------------------------
	void renderCubes(const mat4 & projection, const mat4 & modelview, GLuint uProjection) {

		// cube scaling
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

		// cube movement
		else if (cube_up && cubePos.y < 0.5f) {
			moveCubes(0.0f, 0.01f, 0.0f);
		}
		else if (cube_down&& cubePos.y > -0.5f) {
			moveCubes(0.0f, -0.01f, 0.0f);
		}
		else if (cube_right && cubePos.x < 0.5f) {
			moveCubes(0.01f, 0.0f, 0.0f);
		}
		else if (cube_left && cubePos.x > -0.5f) {
			moveCubes(-0.01f, 0.0f, 0.0f);
		}
		else if (cube_forward && cubePos.z < -1.0f) {
			moveCubes(0.0f, 0.0f, 0.01f);
		}
		else if (cube_backward && cubePos.z > -9.0f) {
			moveCubes(0.0f, 0.0f, -0.01f);
		}
		else if (cube_pos_reset) {
			repositionCubes();
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

		// current projection and head transformation matrix
		projection_curr = projection;
		modelview_curr = glm::mat4(1.0f);
		modelview_curr[3] = modelview[3];
		//modelview_curr = glm::inverse(modelview_curr);
		
		// fix rotation so scene does not rotate with head
		//modelview_curr[0] = modelview_prev[0];
		//modelview_curr[1] = modelview_prev[1];
		//modelview_curr[2] = modelview_prev[2];

		// do not update projection or modelview in freeze mode
		if (freeze_view) {
			//projection_curr = projection_prev;
			modelview_curr = modelview_prev;
		}

		// shader configuration
		glUseProgram(cube_shader);
		GLuint uModel = glGetUniformLocation(cube_shader, "model");

		// render skybox
		mat4 scaleMat = scale(glm::mat4(1.0f), glm::vec3(100.0f, 100.0f, 100.0f));
		glUniformMatrix4fv(uModel, 1, GL_FALSE, &scaleMat[0][0]);

		// render different texture images for left and right eye to create stereo effect
		if (isLeftEye) {
			skybox_left->draw(cube_shader, projection_curr, modelview_curr);
		}
		else {		
			skybox_right->draw(cube_shader, projection_curr, modelview_curr);
		}

		renderCubes(projection_curr, modelview_curr, uModel);

		// store projection and head matrix from the last frame
		projection_prev = projection_curr;
		modelview_prev = modelview_curr;
	};

	// calculate projection matrix for off-screen rendering
	// ---------------------------------------------------
	mat4 getProjectionMatrix(mat4 model, mat4 eyePos) {
		vec3 p_a = (vec3)(model * vec4(PA.x, PA.y, PA.z, 1.0f));
		vec3 p_b = (vec3)(model * vec4(PB.x, PB.y, PB.z, 1.0f));
		vec3 p_c = (vec3)(model * vec4(PC.x, PC.y, PC.z, 1.0f));
		vec3 p_e = (vec3)eyePos[3];

		/*vec3 p_a = vec3(0.0f, -1.0f, -sqrt(2.0f));
		vec3 p_b = vec3(sqrt(2.0f), -1.0f, 0.0f);
		vec3 p_c = vec3(0.0f, 1.0f, -sqrt(2.0f));*/

		/*cout << "p_a: " << p_a.x << ", " << p_a.y << ", " << p_a.z << endl;
		cout << "p_b: " << p_b.x << ", " << p_b.y << ", " << p_b.z << endl;
		cout << "p_c: " << p_c.x << ", " << p_c.y << ", " << p_c.z << endl;
		cout << endl;*/

		// vectors from eye to corners
		vec3 v_a = p_a - p_e;
		vec3 v_b = p_b - p_e;
		vec3 v_c = p_c - p_e;

		/*vec3 v_a = p_c - p_e;
		vec3 v_b = p_b - p_e;
		vec3 v_c = p_a - p_e;*/

		// screen-local axes which give us a basis for describing points relative to the screen
		vec3 v_r = (p_b - p_a)/* / distance(p_b, p_a)*/;
		vec3 v_u = (p_c - p_a)/* / distance(p_c, p_a)*/;
		vec3 v_n = cross(v_r, v_u)/* / length(cross(v_r, v_u))*/;

		//vec3 v_r = (p_b - p_c)/* / distance(p_b, p_a)*/;
		//vec3 v_u = (p_a - p_c)/* / distance(p_c, p_a)*/;
		//vec3 v_n = cross(v_r, v_u)/* / length(cross(v_r, v_u))*/;

		v_r = normalize(v_r);
		v_u = normalize(v_u);
		v_n = normalize(v_n);

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
		M_T[3] = vec4(0.0f, 0.0f, 0.0f, 1.0f);

		// view point offset
		mat4 T = mat4(1.0f);
		T[3] = vec4(-p_e.x, -p_e.y, -p_e.z, 1.0f);

		//mat4 T = translate(mat4(1.0f), -p_e);

		// actual projection that we want to return
		mat4 P_prime = P * M_T * T;

		return P_prime;
	}

	// Rendering scene on screen
	// ---------------------------------------------------
	void renderCave(const mat4 & projection, const mat4 & modelview, bool isLeftEye, GLuint old_FBO) {


		////////////////
		// model matrix for 1st quad
		vec3 pos_1 = vec3(0.0f, /*0.0f*/-3.0f, -8.0f);
		mat4 posMat = translate(mat4(1.0f), pos_1);
		mat4 rotateMat = rotate(mat4(1.0f), 45 * pi<float>() / 180, vec3(0.0f, 1.0f, 0.0f));
		quadModel_1 = posMat * quadScaleMat * rotateMat; // M = T*S*R
														 //quadModel_1 = posMat * mat4(1.0f);

														 // model matrix for 2nd quad
		rotateMat = glm::rotate(mat4(1.0f), -45 * pi<float>() / 180, vec3(0.0f, 1.0f, 0.0f));
		quadModel_2 = posMat * quadScaleMat * rotateMat;

		// model matrix for 3rd quad
		/*rotateMat = glm::rotate(mat4(1.0f), (float)(45 * M_PI) / 180, vec3(0.0f, 0.0f, 1.0f));
		rotateMat = glm::rotate(mat4(1.0f), -(float)(90 * M_PI) / 180, vec3(1.0f, 0.0f, 0.0f)) * rotateMat;*/

		/*rotateMat = glm::rotate(mat4(1.0f), -90 * pi<float>() / 180, vec3(1.0f, 0.0f, 0.0f));
		rotateMat = glm::rotate(mat4(1.0f), 45 * pi<float>() / 180, vec3(0.0f, 1.0f, 0.0f)) * rotateMat;*/

		mat4 T_b = translate(mat4(1.0f), vec3(0.0f, -3.0f, -8.0f));
		rotateMat = /*T_b * */rotate(glm::mat4(1.0f), -1.0f / 2.0f * pi<float>(), vec3(1.0f, 0.0f, 0.0f))
			* rotate(glm::mat4(1.0f), -1.0f / 4.0f * pi<float>(), vec3(0.0f, 0.0f, 1.0f));

		quadModel_3 = T_b /** posMat*/ * quadScaleMat * rotateMat;

		// for each quad, bind to new FBO, renderScene using their P_prime, and bind back to default _fbo
		
		// 1st quad
		glBindFramebuffer(GL_FRAMEBUFFER, FBO); // bind to new framebuffer and draw scene as we normally would to color texture 
		//glEnable(GL_DEPTH_TEST); // enable depth testing
		
		glClearColor(0.4f, 0.5f, 0.3f, 1.0f); // clear the framebuffer's content
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // not using the stencil buffer now

		mat4 P_prime = mat4(1.0f);;
		
		if (head_in_hand) {
			P_prime = getProjectionMatrix(quadModel_1, handMat);
			render(P_prime, handMat, isLeftEye);
		}
		else {
			P_prime = getProjectionMatrix(quadModel_1, modelview/*inverse(modelview)*/);
			render(P_prime, modelview, isLeftEye);
		}
		
		glBindFramebuffer(GL_FRAMEBUFFER, old_FBO);


		// 2nd quad
		glBindFramebuffer(GL_FRAMEBUFFER, FBO_2);

		glClearColor(0.4f, 0.5f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	

		if (head_in_hand) {
			P_prime = getProjectionMatrix(quadModel_2, handMat);
			render(P_prime, handMat, isLeftEye);
		}
		else {
			P_prime = getProjectionMatrix(quadModel_2, modelview/*inverse(modelview)*/);
			render(P_prime, modelview, isLeftEye);
		}

		glBindFramebuffer(GL_FRAMEBUFFER, old_FBO);


		// 3rd quad
		glBindFramebuffer(GL_FRAMEBUFFER, FBO_3);

		glClearColor(0.4f, 0.5f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		if (head_in_hand) {
			P_prime = getProjectionMatrix(quadModel_3, handMat);
			render(P_prime, handMat, isLeftEye);
		}
		else {
			P_prime = getProjectionMatrix(quadModel_3, modelview/*inverse(modelview)*/);
			render(P_prime, modelview, isLeftEye);
		}

		glBindFramebuffer(GL_FRAMEBUFFER, old_FBO);

		//triangle_1->draw(pyramid_shader, P_prime, modelview);

		glClearColor(0.4f, 0.5f, 0.3f, 1.0f); // clear all relevant buffers
	};

	// Rendering screens
	// ---------------------------------------------------
	void renderQuads(const mat4 & projection, const mat4 & modelview, bool isLeft) {

		glDisable(GL_DEPTH_TEST); // disable depth test so screen-space quad isn't discarded due to depth test.

		glUseProgram(plane_shader);
		GLuint uModel = glGetUniformLocation(plane_shader, "model");

		GLuint uTexture = glGetUniformLocation(plane_shader, "screenTexture");

		// draw 1st quad
		glUniformMatrix4fv(uModel, 1, GL_FALSE, &quadModel_1[0][0]);
		plane_1->draw(plane_shader, textureColorbuffer, projection, modelview);

		//////----------------------------------------

		// draw 2nd quad
		glUniformMatrix4fv(uModel, 1, GL_FALSE, &quadModel_2[0][0]);

		// simulate one screen failure on right eye
		if (!isLeft && screen_fail) {
			plane_1->draw(plane_shader, 0, projection, modelview);
		}
		else {
			//plane_1->draw(plane_shader, textureColorbuffer_2, projection, modelview);
			plane_2->draw(plane_shader, textureColorbuffer_2, projection, modelview);
		}
	
		// draw 3rd quad as floor
		glUniformMatrix4fv(uModel, 1, GL_FALSE, &quadModel_3[0][0]);
		//plane_1->draw(plane_shader, textureColorbuffer_3, projection, modelview);
		plane_3->draw(plane_shader, textureColorbuffer_3, projection, modelview);

		////////// temp draw triangles here
		//cout << "resetting vertices for triangle" << endl;
		//vec3 p_e = (vec3)modelview[3];

		vec3 p1_1 = (vec3)(quadModel_1 * vec4(p1, 1.0f));
		vec3 p2_1 = (vec3)(quadModel_1 * vec4(p2, 1.0f));
		vec3 p3_1 = (vec3)(quadModel_1 * vec4(p3, 1.0f));
		vec3 p4_1 = (vec3)(quadModel_1 * vec4(p4, 1.0f));
		vec3 p5_1 = (vec3)(quadModel_1 * vec4(p5, 1.0f));
		vec3 p6_1 = (vec3)(quadModel_1 * vec4(p6, 1.0f));

		vec3 p1_2 = (vec3)(quadModel_2 * vec4(p1, 1.0f));
		vec3 p2_2 = (vec3)(quadModel_2 * vec4(p2, 1.0f));
		vec3 p3_2 = (vec3)(quadModel_2 * vec4(p3, 1.0f));
		vec3 p4_2 = (vec3)(quadModel_2 * vec4(p4, 1.0f));
		vec3 p5_2 = (vec3)(quadModel_2 * vec4(p5, 1.0f));
		vec3 p6_2 = (vec3)(quadModel_2 * vec4(p6, 1.0f));

		vec3 p1_3 = (vec3)(quadModel_3 * vec4(p1, 1.0f));
		vec3 p2_3 = (vec3)(quadModel_3 * vec4(p2, 1.0f));
		vec3 p3_3 = (vec3)(quadModel_3 * vec4(p3, 1.0f));
		vec3 p4_3 = (vec3)(quadModel_3 * vec4(p4, 1.0f));
		vec3 p5_3 = (vec3)(quadModel_3 * vec4(p5, 1.0f));
		vec3 p6_3 = (vec3)(quadModel_3 * vec4(p6, 1.0f));

		vec3 p_e = (vec3)(handMat[3]);

		if (debug_mode && head_in_hand) {

			triangle_1->setVertices(p_e, p1_1, p2_1);
			triangle_2->setVertices(p_e, p1_2, p2_2);
			triangle_3->setVertices(p_e, p1_3, p2_3);

			triangle_1->draw(pyramid_shader, projection, modelview, quadModel_1, isLeft);
			//triangle_2->draw(pyramid_shader, projection, modelview, quadModel_2, isLeft);
			//triangle_3->draw(pyramid_shader, projection, modelview, quadModel_3, isLeft);

			triangle_1->setVertices(p_e, p1_1, p3_1);
			triangle_2->setVertices(p_e, p1_2, p3_2);
			triangle_3->setVertices(p_e, p1_3, p3_3);

			//triangle_1->draw(pyramid_shader, projection, modelview, quadModel_1, isLeft);
			//triangle_2->draw(pyramid_shader, projection, modelview, quadModel_2, isLeft);
			//triangle_3->draw(pyramid_shader, projection, modelview, quadModel_3, isLeft);

			triangle_1->setVertices(p_e, p1_1, p4_1);
			triangle_2->setVertices(p_e, p1_2, p4_2);
			triangle_3->setVertices(p_e, p1_3, p4_3);

			//triangle_1->draw(pyramid_shader, projection, modelview, quadModel_1, isLeft);
			//triangle_2->draw(pyramid_shader, projection, modelview, quadModel_2, isLeft);
			//triangle_3->draw(pyramid_shader, projection, modelview, quadModel_3, isLeft);

			triangle_1->setVertices(p_e, p1_1, p5_1);
			triangle_2->setVertices(p_e, p1_2, p5_2);
			triangle_3->setVertices(p_e, p1_3, p5_3);

			//triangle_1->draw(pyramid_shader, projection, modelview, quadModel_1, isLeft);
			//triangle_2->draw(pyramid_shader, projection, modelview, quadModel_2, isLeft);
			//triangle_3->draw(pyramid_shader, projection, modelview, quadModel_3, isLeft);

			triangle_1->setVertices(p_e, p1_1, p6_1);
			triangle_2->setVertices(p_e, p1_2, p6_2);
			triangle_3->setVertices(p_e, p1_3, p6_3);

			//triangle_1->draw(pyramid_shader, projection, modelview, quadModel_1, isLeft);
			//triangle_2->draw(pyramid_shader, projection, modelview, quadModel_2, isLeft);
			//triangle_3->draw(pyramid_shader, projection, modelview, quadModel_3, isLeft);

			////////////////////////////////////////

			triangle_1->setVertices(p_e, p2_1, p3_1);
			triangle_2->setVertices(p_e, p2_2, p3_2);
			triangle_3->setVertices(p_e, p2_3, p3_3);

			triangle_1->draw(pyramid_shader, projection, modelview, quadModel_1, isLeft);
			//triangle_2->draw(pyramid_shader, projection, modelview, quadModel_2, isLeft);
			//triangle_3->draw(pyramid_shader, projection, modelview, quadModel_3, isLeft);

			triangle_1->setVertices(p_e, p2_1, p4_1);
			triangle_2->setVertices(p_e, p2_2, p4_2);
			triangle_3->setVertices(p_e, p2_3, p4_3);

			//triangle_1->draw(pyramid_shader, projection, modelview, quadModel_1, isLeft);
			//triangle_2->draw(pyramid_shader, projection, modelview, quadModel_2, isLeft);
			triangle_3->draw(pyramid_shader, projection, modelview, quadModel_3, isLeft);

			triangle_1->setVertices(p_e, p2_1, p5_1);
			triangle_2->setVertices(p_e, p2_2, p5_2);
			triangle_3->setVertices(p_e, p2_3, p5_3);

			//triangle_1->draw(pyramid_shader, projection, modelview, quadModel_1, isLeft);
			triangle_2->draw(pyramid_shader, projection, modelview, quadModel_2, isLeft);
			//triangle_3->draw(pyramid_shader, projection, modelview, quadModel_3, isLeft);

			triangle_1->setVertices(p_e, p2_1, p6_1);
			triangle_2->setVertices(p_e, p2_2, p6_2);
			triangle_3->setVertices(p_e, p2_3, p6_3);

			//triangle_1->draw(pyramid_shader, projection, modelview, quadModel_1, isLeft);
			//triangle_2->draw(pyramid_shader, projection, modelview, quadModel_2, isLeft);
			//triangle_3->draw(pyramid_shader, projection, modelview, quadModel_3, isLeft);

			////////////////////////////////////////

			triangle_1->setVertices(p_e, p3_1, p4_1);
			triangle_2->setVertices(p_e, p3_2, p4_2);
			triangle_3->setVertices(p_e, p3_3, p4_3);

			triangle_1->draw(pyramid_shader, projection, modelview, quadModel_1, isLeft);
			//triangle_2->draw(pyramid_shader, projection, modelview, quadModel_2, isLeft);
			//triangle_3->draw(pyramid_shader, projection, modelview, quadModel_3, isLeft);

			triangle_1->setVertices(p_e, p3_1, p5_1);
			triangle_2->setVertices(p_e, p3_2, p5_2);
			triangle_3->setVertices(p_e, p3_3, p5_3);

			//triangle_1->draw(pyramid_shader, projection, modelview, quadModel_1, isLeft);
			triangle_2->draw(pyramid_shader, projection, modelview, quadModel_2, isLeft);
			//triangle_3->draw(pyramid_shader, projection, modelview, quadModel_3, isLeft);

			triangle_1->setVertices(p_e, p3_1, p6_1);
			triangle_2->setVertices(p_e, p3_2, p6_2);
			triangle_3->setVertices(p_e, p3_3, p6_3);

			//triangle_1->draw(pyramid_shader, projection, modelview, quadModel_1, isLeft);
			//triangle_2->draw(pyramid_shader, projection, modelview, quadModel_2, isLeft);
			//triangle_3->draw(pyramid_shader, projection, modelview, quadModel_3, isLeft);

			/////////////////////////////////////////

			triangle_1->setVertices(p_e, p4_1, p5_1);
			triangle_2->setVertices(p_e, p4_2, p5_2);
			triangle_3->setVertices(p_e, p4_3, p5_3);

			//triangle_1->draw(pyramid_shader, projection, modelview, quadModel_1, isLeft);
			//triangle_2->draw(pyramid_shader, projection, modelview, quadModel_2, isLeft);
			//triangle_3->draw(pyramid_shader, projection, modelview, quadModel_3, isLeft);

			triangle_1->setVertices(p_e, p4_1, p6_1);
			triangle_2->setVertices(p_e, p4_2, p6_2);
			triangle_3->setVertices(p_e, p4_3, p6_3);

			//triangle_1->draw(pyramid_shader, projection, modelview, quadModel_1, isLeft);
			//triangle_2->draw(pyramid_shader, projection, modelview, quadModel_2, isLeft);
			//triangle_3->draw(pyramid_shader, projection, modelview, quadModel_3, isLeft);

			/////////////////////////////////////////

			triangle_1->setVertices(p_e, p5_1, p6_1);
			triangle_2->setVertices(p_e, p5_2, p6_2);
			triangle_3->setVertices(p_e, p5_3, p6_3);

			//triangle_1->draw(pyramid_shader, projection, modelview, quadModel_1, isLeft);
			//triangle_2->draw(pyramid_shader, projection, modelview, quadModel_2, isLeft);
			//triangle_3->draw(pyramid_shader, projection, modelview, quadModel_3, isLeft);

		}

		
		
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