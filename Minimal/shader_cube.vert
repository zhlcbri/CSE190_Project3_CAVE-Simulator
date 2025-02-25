#version 330 core
layout (location = 0) in vec3 aPos;

out vec3 TexCoords;

uniform mat4 projection; //
uniform mat4 view; // modelView

uniform mat4 model; // used for scaling

void main()
{       
    TexCoords = aPos;
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}
