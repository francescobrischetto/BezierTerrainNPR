#version 410 core

// vertex position
layout (location = 0) in vec3 position;
// vertex normal
layout (location = 1) in vec3 normal;

// Trimesh useful mesh properties

// Principal curvatures values
layout (location = 2) in float curv1;
layout (location = 3) in float curv2;
// Mesh feature size -> median edge length
layout (location = 4) in float feature_size;
// Principal curvature directions
layout (location = 5) in vec3 pdir1;
layout (location = 6) in vec3 pdir2;
// Directional derivatives in Principal Curvature Directions (Taubin's Algorithm)
layout (location = 7) in vec4 dcurv;

//Uniforms
// model matrix
uniform mat4 modelMatrix;

// Structure to pass data to Tessellation Control Shader
out VS_OUT{
    vec3 position;
    vec3 normal;
    float curv1;
    float curv2;
    float feature_size;
    vec3 pdir1;
    vec3 pdir2;
    vec4 dcurv;
} tcs_in;

void main(){
    // Position in world space
    tcs_in.position = (modelMatrix * vec4(position,1.0)).xyz;
    // Other values are passed to the tcs
    tcs_in.normal = normal;
    tcs_in.curv1 = curv1;
    tcs_in.curv2 = curv2;
    tcs_in.feature_size = feature_size;
    tcs_in.pdir1 = pdir1;
    tcs_in.pdir2 = pdir2;
    tcs_in.dcurv = dcurv;
}
