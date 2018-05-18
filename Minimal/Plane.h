#pragma once
#ifndef Plane_h
#define Plane_h

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

#include "stb_image.h"

using namespace std;

float quadVertices[18] = {

	// first triangle
	-1.0f, -1.0f, -1.0f,
	1.0f, -1.0f,-1.0f,
	-1.0f, 1.0f, -1.0f,

	// second triangle
	-1.0f, 1.0f,-1.0f,
	1.0f, -1.0f, -1.0f,
	1.0f, 1.0f, -1.0f,

	//-1.0f,  1.0f, -1.0f,
	//-1.0f, -1.0f, -1.0f,
	//1.0f, -1.0f, -1.0f,

	//-1.0f,  1.0f, -1.0f,
	//1.0f, -1.0f, -1.0f,
	//1.0f,  1.0f, -1.0f,
};

class Plane {
private:
	//int size = 1;

public:
	GLsizei WIDTH = 1000;
	GLsizei HEIGHT = 1000;

	GLuint quadVAO, quadVBO;
	GLuint uProjection, uModelview;
	GLuint myFBO;
	GLuint myTexBuffer;
	GLuint myRBO;

	Plane(GLuint yourFBO, GLuint yourTexBuffer, GLuint yourRBO) {
		myFBO = yourFBO;
		myTexBuffer = yourTexBuffer;
		myRBO = yourRBO;

		// screen quad VAO
		glGenVertexArrays(1, &quadVAO);
		glGenBuffers(1, &quadVBO);

		glBindVertexArray(quadVAO);
		glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
		//glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), &planeVertices, GL_STATIC_DRAW);

		glEnableVertexAttribArray(0);
		//glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (GLvoid*)0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (GLvoid*)0);
		
		glEnableVertexAttribArray(1);
		//glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (GLvoid*)(2 * sizeof(float)));
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (GLvoid*)(2 * sizeof(float)));

		// Unbind the currently bound buffer so that we don't accidentally make unwanted changes to it.
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);

		////////////////////

		//glGenFramebuffers(1, &myFBO);
		//glBindFramebuffer(GL_FRAMEBUFFER, myFBO);

		//// create color attachment textures
		//glGenTextures(1, &myTexBuffer);
		//glBindTexture(GL_TEXTURE_2D, myTexBuffer);

		//glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, WIDTH, HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL); // TEXTURE_WIDTH?
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		//// to render whole screen to a texture call glViewport() before rendering to framebuffer with the new dimensions of texture
		//glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, myTexBuffer, 0);

		//// create a renderbuffer object for depth and stencil attachment (we won't be sampling these)
		//glGenRenderbuffers(1, &myRBO);
		//glBindRenderbuffer(GL_RENDERBUFFER, myRBO);
		//// use a single renderbuffer object for both a depth AND stencil buffer.
		//glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, WIDTH, HEIGHT);

		//// now actually attach it
		//glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, myRBO);

		//// now that we actually created the framebuffer and added all attachments we want to check if it is actually complete now
		//if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		//	cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << endl;
		//glBindFramebuffer(GL_FRAMEBUFFER, 0);
	};

	~Plane() {
		// screen quad VAO
		glDeleteBuffers(1, &quadVAO);
		glDeleteBuffers(1, &quadVBO);
	}

	void draw(GLuint shaderProgram, GLuint texColorbuffer, const glm::mat4 & projection, const glm::mat4 & modelview) {
		//glDisable(GL_CULL_FACE);

		glUseProgram(shaderProgram);

		uProjection = glGetUniformLocation(shaderProgram, "projection");
		uModelview = glGetUniformLocation(shaderProgram, "view");

		// send these values to the shader program
		glUniformMatrix4fv(uProjection, 1, GL_FALSE, &projection[0][0]);
		glUniformMatrix4fv(uModelview, 1, GL_FALSE, &modelview[0][0]);

		glBindVertexArray(quadVAO);

		// use the color attachment texture as the texture of the quad plane
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D,/* myTexBuffer*/texColorbuffer);
		glUniform1i(glGetUniformLocation(shaderProgram, "screenTexture"), 0);

		// Draw triangles
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glBindVertexArray(0);
	};

	//float quadVertices[24] = { // vertex attributes for a quad that fills the entire screen in Normalized Device Coordinates.
	//						   // positions   // texCoords
	//	-1.0f,  1.0f,  0.0f, 1.0f,
	//	-1.0f, -1.0f,  0.0f, 0.0f,
	//	1.0f, -1.0f,  1.0f, 0.0f,

	//	-1.0f,  1.0f,  0.0f, 1.0f,
	//	1.0f, -1.0f,  1.0f, 0.0f,
	//	1.0f,  1.0f,  1.0f, 1.0f
	//};

};

#endif