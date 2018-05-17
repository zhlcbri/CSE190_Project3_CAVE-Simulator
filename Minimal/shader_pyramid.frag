#version 330 core

out vec4 FragColor;

uniform int color_mode; // 0 for green, 1 for red

void main()
{
    if (color_mode == 0) {
	  FragColor = vec4(0.0f, 0.5f, 0.0f, 1.0f);
	}
    else {
	  FragColor = vec4(0.5f, 0.0f, 0.0f, 1.0f);	  
	}
} 
