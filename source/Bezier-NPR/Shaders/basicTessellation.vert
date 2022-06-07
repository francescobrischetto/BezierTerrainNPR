#version 410 core

// vertex position in world coordinates
layout (location = 0) in vec3 position;
// vertex normal in world coordinate
layout (location = 1) in vec3 normal;

out vec3 tcsPos;
out vec3 tcsNormal;
// model matrix
uniform mat4 modelMatrix;

void main(){
    tcsPos = (modelMatrix * vec4(position,1.0)).xyz;
    tcsNormal = (modelMatrix * vec4(normal,0.0)).xyz;
}
