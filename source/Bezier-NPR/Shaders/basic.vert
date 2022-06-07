/*

02_flatten.vert: the Vertex shader modifies one of the vertex coordinates, "flattening" the final model

author: Davide Gadia

Real-Time Graphics Programming - a.a. 2020/2021
Master degree in Computer Science
Universita' degli Studi di Milano

*/

#version 410 core

// vertex position in world coordinates
// the number used for the location in the layout qualifier is the position of the vertex attribute
// as defined in the Mesh class
layout (location = 0) in vec3 position;

// model matrix
uniform mat4 modelMatrix;
// view matrix
uniform mat4 viewMatrix;
// Projection matrix
uniform mat4 projectionMatrix;

void main()
{
	// Original vertex position is copied in a local variable.
	vec3 flattened = position;
	// Z coordinate is set at 0
	//flattened.z = 0.0;
	// transformations are applied to the modified vertex
	gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4( flattened, 1.0 );
}
