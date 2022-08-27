#version 410 core

layout (location = 0) in vec3 position;

void main()
{
    //Simple passing Vertex Position to Tessellation Control Shader
    gl_Position = vec4(position, 1.0);

}