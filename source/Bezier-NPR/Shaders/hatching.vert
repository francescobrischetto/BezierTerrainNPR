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
layout (location = 1) in vec3 normal;

// model matrix
uniform mat4 modelMatrix;
// view matrix
uniform mat4 viewMatrix;
// Projection matrix
uniform mat4 projectionMatrix;
uniform mat3 normalMatrix;
uniform vec3 pointLightPosition;

out vec3 vPosition;
out float nDotVP;
out vec3 vNormal;


void main()
{
	float w = 1.;
	vec3 posInc = vec3( 0. );
	posInc = w * normal;
	vec4 mvPosition = viewMatrix * modelMatrix * vec4( position, 1.0 );
	vPosition = mvPosition.xyz;
	gl_Position = projectionMatrix * mvPosition;
	vNormal = normalMatrix * normal;
	nDotVP = max( 0., dot( vNormal, normalize( ( viewMatrix * vec4(pointLightPosition, 1.0) ).xyz ) ) );

	//vec4 lightPos = viewMatrix  * vec4(pointLightPosition, 1.0);
}