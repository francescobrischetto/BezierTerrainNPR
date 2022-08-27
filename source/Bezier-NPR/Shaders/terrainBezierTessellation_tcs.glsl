#version 410 core
// Define the number of Control Points in the output patch
layout (vertices = 16) out;

// Necessary Matrices to calculate distance from camera
uniform mat4 modelMatrix;
uniform mat4 viewMatrix;

float getTessellationLevelBasedOnCameraDistance(vec4 currentPointPosition){
    // Tessellation Parameters
    int minTessLevel = 2;       int maxTessLevel = 8;
    float maxDepthToConsider = 1000.0;    float minDepthToConsider = 100.0;
    // Position in camera Coordinates
    vec4 positionV = viewMatrix * modelMatrix * currentPointPosition;
    // Tessellation level based on distance from camera
    // "Distance" from camera scaled between 0 and 1
    float currScaledDepth = clamp( (abs(positionV.z) - minDepthToConsider) / (maxDepthToConsider - minDepthToConsider),0.0, 1.0 );
    // Interpolate between min/max tess levels
    float tessLevel = mix(maxTessLevel, minTessLevel, currScaledDepth);
    return tessLevel;
}

void main()
{
    vec4 currentPointPosition = gl_in[gl_InvocationID].gl_Position;
    float tessLevel = getTessellationLevelBasedOnCameraDistance(currentPointPosition);
    // For quads we have 4 tessellation Outer levels and 2 Tessellation inner levels.
    // We set them all to the calculated tessellation level based on the distance from camera.
    gl_TessLevelOuter[0] = tessLevel;
    gl_TessLevelOuter[1] = tessLevel;
    gl_TessLevelOuter[2] = tessLevel;
    gl_TessLevelOuter[3] = tessLevel;
    gl_TessLevelInner[0] = tessLevel;
    gl_TessLevelInner[1] = tessLevel;
    // We also pass the position to the Tessellation Evaluation Shader
    gl_out[gl_InvocationID].gl_Position = currentPointPosition;

}



