#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec2 textPos;
layout(location = 2) in vec3 aNormal;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec2 texCoord;
out vec3 Normal;
out vec3 forgPos;
void main() 
{
	gl_Position =  projection * view * model * vec4(aPos, 1.0);
	forgPos = vec3(model * vec4(aPos, 1.0));
	texCoord = textPos;
	Normal = mat3(transpose(inverse(model))) * aNormal;
}