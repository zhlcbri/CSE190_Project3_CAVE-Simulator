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

#include <GL/glew.h>

#define STB_IMAGE_IMPLEMENTATION
#define _USE_MATH_DEFINES
#include <math.h>

#include "stb_image.h"
#include "shader.h"

using namespace std;
using namespace glm;

class Cave {
private:

public:
	Cube * cube_1;
	Cube * skybox_left;
	Cube * skybox_right;
	Cube * skybox_room;
	Cube * controller;
	Plane * plane_1;

	GLuint cube_shader;
	GLuint plane_shader;
	GLuint FBO, textureColorbuffer, rbo;

	GLuint tempTex; // temp

	GLsizei WIDTH = 1280;
	GLsizei HEIGHT = 720;

	glm::mat4 headPos_curr = glm::mat4(1.0f);
	glm::mat4 headPos_prev = glm::mat4(1.0f);

	/*glm::vec3 hand;*/

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

	glm::mat4 cubeScaleMat = glm::scale(glm::mat4(1.0f), glm::vec3(0.3f, 0.3f, 0.3f)); // only mat used to scale cube

	glm::mat4 quadScaleMat = glm::scale(glm::mat4(1.0f), glm::vec3(8.0f, 8.0f, 8.0f)); // only mat used to scale quad

	Cave() {
		skybox_left = new Cube(1, skybox_faces_left, true, true, false);
		skybox_right = new Cube(1, skybox_faces_right, true, false, false);
		skybox_room = new Cube(1, skybox_faces_room, true, false, true);
		cube_1 = new Cube(1, cube_faces, false, false, false); // calibration cube
		controller = new Cube(1, cube_faces, false, false, false); // controller
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
		
		/*glGenerateMipmap(GL_TEXTURE_2D);*/

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, /*GL_TEXTURE_WIDTH*/WIDTH, /*GL_TEXTURE_HEIGHT*/HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL); // TEXTURE_WIDTH?
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		//glBindTexture(GL_TEXTURE_2D, 0); //

		/*glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);*/

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
		cubeScaleMat = cubeScaleMat * glm::scale(glm::mat4(1.0f), glm::vec3(val));
	};

	////////////////////
	void renderCubes(const mat4 & projection, const mat4 & modelview, GLuint uProjection) {
		// change cubeScaleMat according to booleans

		// render cubes
		// specify positions
		vec3 pos_1 = vec3(0.0f, 0.0f, -4.0f);
		vec3 pos_2 = vec3(0.0f, 0.0f, -8.0f);

		glm::mat4 posMat = glm::translate(glm::mat4(1.0f), pos_1);
		glm::mat4 posMat_in = glm::translate(glm::mat4(1.0f), -pos_1);
		glm::mat4 M = posMat * cubeScaleMat * posMat_in;

		// draw closer cube
		glUniformMatrix4fv(uProjection, 1, GL_FALSE, &M[0][0]);
		cube_1->draw(cube_shader, projection, modelview);

		posMat = glm::translate(glm::mat4(1.0f), pos_2);
		posMat_in = glm::translate(glm::mat4(1.0f), -pos_2);
		M = posMat * cubeScaleMat * posMat_in;

		// draw further cube
		glUniformMatrix4fv(uProjection, 1, GL_FALSE, &M[0][0]);
		cube_1->draw(cube_shader, projection, modelview);
	};

	////////////////////
	void renderQuads(const mat4 & projection, const mat4 & modelview, GLuint uModel) {

		// specify positions
		vec3 pos_1 = vec3(0.0f, 0.0f, -8.0f);
		glm::mat4 posMat = glm::translate(glm::mat4(1.0f), pos_1);	
		//glm::mat4 posMat_in = glm::translate(glm::mat4(1.0f), -pos_1);

		mat4 rotateMat = glm::rotate(mat4(1.0f), (float)(45 * M_PI) / 180, vec3(0.0f, 1.0f, 0.0f));

		//glm::mat4 M = posMat *  quadScaleMat * posMat_in;
		glm::mat4 M = posMat * quadScaleMat * rotateMat;

		// draw 1st quad
		glUniformMatrix4fv(uModel, 1, GL_FALSE, &M[0][0]);
		plane_1->draw(plane_shader, textureColorbuffer, projection, modelview);


		// draw 2nd quad
		rotateMat = glm::rotate(mat4(1.0f), -(float)(45 * M_PI) / 180, vec3(0.0f, 1.0f, 0.0f));
		M = posMat * quadScaleMat * rotateMat;
		glUniformMatrix4fv(uModel, 1, GL_FALSE, &M[0][0]);
		plane_1->draw(plane_shader, textureColorbuffer, projection, modelview);


		// draw 3rd quad as floor
		rotateMat = glm::rotate(mat4(1.0f), (float)(45 * M_PI) / 180, vec3(0.0f, 0.0f, 1.0f));
		rotateMat = glm::rotate(mat4(1.0f), -(float)(90 * M_PI) / 180, vec3(1.0f, 0.0f, 0.0f)) * rotateMat;
		M = posMat * quadScaleMat * rotateMat;
		glUniformMatrix4fv(uModel, 1, GL_FALSE, &M[0][0]);
		plane_1->draw(plane_shader, textureColorbuffer, projection, modelview);

	};

	void renderController(const mat4 & projection, const mat4 & modelview, vec3 handPos) {
		// render cube at left controller
		// specify positions

		glm::mat4 posMat = glm::translate(glm::mat4(1.0f), handPos);
		glm::mat4 posMat_in = glm::translate(glm::mat4(1.0f), -handPos);

		glm::mat4 scaleMat = glm::scale(glm::mat4(1.0f), glm::vec3(0.03f, 0.03f, 0.03f));

		glm::mat4 M = posMat * scaleMat * posMat_in;

		// draw cube at controller position
		glUseProgram(cube_shader);
		GLuint uProjection = glGetUniformLocation(cube_shader, "model");
		glUniformMatrix4fv(uProjection, 1, GL_FALSE, &M[0][0]);
		controller->draw(cube_shader, projection, modelview);
	};

	// render skybox and cubes
	void render(const mat4 & projection, const mat4 & modelview, bool isLeftEye) {

		// freeze head orientation
		headPos_curr = modelview;

		headPos_curr[0] = headPos_prev[0];
		headPos_curr[1] = headPos_prev[1];
		headPos_curr[2] = headPos_prev[2];

		// shader configuration
		glUseProgram(cube_shader);
		GLuint uModel = glGetUniformLocation(cube_shader, "model");

		// render skybox
		glm::mat4 scaleMat = glm::scale(glm::mat4(1.0f), glm::vec3(100.0f, 100.0f, 100.0f));
		glUniformMatrix4fv(uModel, 1, GL_FALSE, &scaleMat[0][0]);

		// render different texture images for left and right eye to create stereo effect
		if (isLeftEye) {
			//cout << "drawing left box" << endl;

			skybox_left->draw(cube_shader, projection, /*modelview)*/headPos_curr);
		}
		else {
			//cout << "drawing right box" << endl;
			
			skybox_right->draw(cube_shader, projection, /*modelview)*/headPos_curr);
		}

		renderCubes(projection, /*modelview)*/headPos_curr, uModel);

		// store head position from the last frame
		headPos_prev = headPos_curr;
	};

	/////////////////////
	void renderCave(const mat4 & projection, const mat4 & modelview, bool isLeftEye, GLuint old_FBO) {

		// bind to new framebuffer and draw scene as we normally would to color texture 
		glBindFramebuffer(GL_FRAMEBUFFER, FBO);
		//glEnable(GL_DEPTH_TEST); // enable depth testing (disabled for rendering screen-space quad)
		
		// clear the framebuffer's content
		glClearColor(0.4f, 0.5f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // we're not using the stencil buffer now

		// draw skybox and cubes
		render(projection,modelview, isLeftEye);

		// now bind back to default framebuffer and draw a quad plane with the attached framebuffer color texture
		glBindFramebuffer(GL_FRAMEBUFFER, old_FBO);
		//glDisable(GL_DEPTH_TEST); // disable depth test so screen-space quad isn't discarded due to depth test.

		// draw plane
		glUseProgram(plane_shader);
		GLuint uModel = glGetUniformLocation(plane_shader, "model");

		renderQuads(projection, modelview, uModel);
		
		// clear all relevant buffers
		glClearColor(0.4f, 0.5f, 0.3f, 1.0f); // set clear color
		//glClear(GL_COLOR_BUFFER_BIT);

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