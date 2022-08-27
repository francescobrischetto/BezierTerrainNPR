#version 410 core

layout (location = 0) in vec3 position;

void main()
{
    //Passing position to tcs
    gl_Position = vec4(position, 1.0);

}