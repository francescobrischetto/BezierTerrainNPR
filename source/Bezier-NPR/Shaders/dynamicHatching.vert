#version 410 core

layout(location = 0) in vec3 vertPos;
layout(location = 1) in vec3 vertNor;

// model matrix
uniform mat4 modelMatrix;
// view matrix
uniform mat4 viewMatrix;
// Projection matrix
uniform mat4 projectionMatrix;
uniform vec3 pointLightPosition;

out vec4 fragPos;
out vec3 fragNor;
out vec3 lightDir;
out vec3 vViewPosition;


void main()
{
    vec4 mvPosition = viewMatrix * modelMatrix * vec4( vertPos, 1.0 );
    vViewPosition = -mvPosition.xyz;
    fragPos = modelMatrix * vec4(vertPos, 1.0);
    fragNor = ( modelMatrix * vec4( vertNor, 0.0 ) ).xyz;
    gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4(vertPos, 1.0);
    vec4 lightPos = viewMatrix  * vec4(pointLightPosition, 1.0);
    lightDir = lightPos.xyz - mvPosition.xyz;
}