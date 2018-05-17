#version 330 core
//layout (location = 0) in vec2 aPos;

layout (location = 0) in vec3 aPos;

layout (location = 1) in vec2 aTexCoords;

out vec2 TexCoords;

uniform mat4 projection; //
uniform mat4 view; // modelView

uniform mat4 model; // used for scaling

void main()
{
    //TexCoords = aTexCoords;

	//gl_Position = projection * view * model * vec4(aPos.x, aPos.y, -1.0, 1.0);

	TexCoords = vec2(aPos.x / 2 + 0.5, aPos.y / 2 + 0.5);
	gl_Position = projection * view * model * vec4(aPos.x, aPos.y, aPos.z, 1.0);

}  