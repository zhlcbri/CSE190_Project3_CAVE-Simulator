#ifndef Triangle_h
#define Triangle_h

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

//mat4 triangle_M = mat4(1.0f);
//mat4 triangle_S = scale(mat4(1.0f), vec3(1.0f, 1.0f, 1.0f));

//float vertices[/*18*/9] = {
//	// first triangle
//	0.5f,  0.5f, 0.0f,  // top right
//	0.5f, -0.5f, 0.0f,  // bottom right
//	-0.5f,  0.5f, 0.0f,  // top left 
//
//	//// second triangle
//	//0.5f, -0.5f, 0.0f,  // bottom right
//	//-0.5f, -0.5f, 0.0f,  // bottom left
//	//-0.5f,  0.5f, 0.0f   // top left
//};


float vertices[] = {
	0.5f,  0.5f, 0.0f,  // top right
	0.5f, -0.5f, 0.0f,  // bottom right
	-0.5f, -0.5f, 0.0f,  // bottom left
	-0.5f,  0.5f, 0.0f   // top left 
};
unsigned int indices[] = {  // note that we start from 0!
	0, 1, 3,   // first triangle
	1, 2, 3    // second triangle
};

class Triangle {
private:
	float myVertices[9] = { 0 };

public:
	GLuint triangleVAO;
	GLuint triangleVBO;
	GLuint triangleEBO;

	// Constructor
	// ---------------------------------------------------
	//Triangle(float myVertices[/*18*/9]) {
	Triangle(float v[9]) {
		
		// populate vertex array
		for (int i = 0; i < sizeof(v); i++) {
			myVertices[i] = v[i];
		}

		glGenVertexArrays(1, &triangleVAO);
		glGenBuffers(1, &triangleVBO);
		glGenBuffers(1, &triangleEBO);

		glBindVertexArray(triangleVAO);

		glBindBuffer(GL_ARRAY_BUFFER, triangleVBO);
		glBufferData(GL_ARRAY_BUFFER, 
			sizeof(myVertices)/*sizeof(vertices)*/, 
			&myVertices/*&(vertices)*/, 
			GL_STATIC_DRAW);

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (GLvoid*)0);
		
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, triangleEBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);

		// temp
		/*vec3 pos_1 = vec3(0.0f, 0.0f, -5.0f);
		mat4 triangle_T = translate(mat4(1.0f), pos_1);
		triangle_M = triangle_T * triangle_S;*/
	}

	// reset vertices so we can draw multiple triangles using one object
	void setVertices(vec3 p1, vec3 p2, vec3 p3) {
		myVertices[0] = p1.x;
		myVertices[1] = p1.y;
		myVertices[2] = p1.z;

		myVertices[3] = p2.x;
		myVertices[4] = p2.y;
		myVertices[5] = p2.z;

		myVertices[6] = p3.x;
		myVertices[7] = p3.y;
		myVertices[8] = p3.z;

		glBindVertexArray(triangleVAO);
		glBindBuffer(GL_ARRAY_BUFFER, triangleVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(myVertices), &myVertices, GL_STATIC_DRAW);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (GLvoid*)0);
		glEnableVertexAttribArray(0);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
	}

	// Destructor
	// ---------------------------------------------------
	~Triangle() {
		glDeleteBuffers(1, &triangleVAO);
		glDeleteBuffers(1, &triangleVBO);
	}

	void draw(GLuint shaderProgram, const mat4 & projection, const mat4 & view, const mat4 & model, bool isLeftEye) {
		glUseProgram(shaderProgram);

		GLuint uProjection = glGetUniformLocation(shaderProgram, "projection");
		GLuint uView = glGetUniformLocation(shaderProgram, "view");
		GLuint uModel = glGetUniformLocation(shaderProgram, "model");
		GLuint uColorMode = glGetUniformLocation(shaderProgram, "color_mode"); // 0 for green; 1 for red

		glUniformMatrix4fv(uProjection, 1, GL_FALSE, &projection[0][0]);
		glUniformMatrix4fv(uView, 1, GL_FALSE, &view[0][0]);
		glUniformMatrix4fv(uModel, 1, GL_FALSE, &model[0][0]);
		
		if (isLeftEye) {
			glUniform1i(uColorMode, /*0*/1);
		}
		else {
			glUniform1i(uColorMode, 1);
		}
		

		glBindVertexArray(triangleVAO);
		
		glLineWidth(6.0f);
		//glDrawElements(GL_LINES, 6, GL_UNSIGNED_INT, 0);
		
		glDrawArrays(GL_TRIANGLES, 0, 3);
		glBindVertexArray(0);
	}
};

#endif