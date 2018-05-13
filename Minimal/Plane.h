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

class Plane {
private:
	int size = 1;

public:
	GLuint quadVAO, quadVBO;

	//GLuint FBO, textureColorbuffer;

	Plane(/*int mySize, GLuint textureID*/) {
		/*size = mySize;
		textureColorbuffer = textureID;*/

		// screen quad VAO
		glGenVertexArrays(1, &quadVAO);
		glGenBuffers(1, &quadVBO);
		glBindVertexArray(quadVAO);
		glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
		//glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), &planeVertices, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
		
		// glBindRenderbuffer(GL_RENDERBUFFER, 0);
	};

	~Plane() {
		// screen quad VAO
		glDeleteBuffers(1, &quadVAO);
		glDeleteBuffers(1, &quadVBO);
	}


	void draw(GLuint shaderProgram, GLuint texColorbuffer) {
		//glDisable(GL_CULL_FACE); //

		glUseProgram(shaderProgram);
		glBindVertexArray(quadVAO);
		//glBindTexture(GL_TEXTURE_2D, texColorbuffer); // use the color attachment texture as the texture of the quad plane
		
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texColorbuffer);
		glUniform1i(glGetUniformLocation(shaderProgram, "screenTexture"), 0);
		
		glDrawArrays(GL_TRIANGLES, 0, 6);
	};


	float quadVertices[24] = { // vertex attributes for a quad that fills the entire screen in Normalized Device Coordinates.
							   // positions   // texCoords
		-1.0f,  1.0f,  0.0f, 1.0f,
		-1.0f, -1.0f,  0.0f, 0.0f,
		1.0f, -1.0f,  1.0f, 0.0f,

		-1.0f,  1.0f,  0.0f, 1.0f,
		1.0f, -1.0f,  1.0f, 0.0f,
		1.0f,  1.0f,  1.0f, 1.0f
	};

	/*float quadVertices[24] = {
		-100.0f,  100.0f,  0.0f, 100.0f,
		-100.0f, -100.0f,  0.0f, 0.0f,
		100.0f, -100.0f,  100.0f, 0.0f,

		-100.0f,  100.0f,  0.0f, 100.0f,
		100.0f, -100.0f,  100.0f, 0.0f,
		100.0f,  100.0f,  100.0f, 100.0f
	};*/

	// prob won't need
	float planeVertices[30] = {
		// positions          // texture Coords 
		5.0f, -0.5f,  5.0f,  2.0f, 0.0f,
		-5.0f, -0.5f,  5.0f,  0.0f, 0.0f,
		-5.0f, -0.5f, -5.0f,  0.0f, 2.0f,

		5.0f, -0.5f,  5.0f,  2.0f, 0.0f,
		-5.0f, -0.5f, -5.0f,  0.0f, 2.0f,
		5.0f, -0.5f, -5.0f,  2.0f, 2.0f
	};

};


#endif