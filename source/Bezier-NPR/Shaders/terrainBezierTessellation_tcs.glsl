#version 410 core
// define the number of CPs in the output patch
layout (vertices = 16) out;

int MinTessLevel = 1;
int MaxTessLevel = 8;
float MaxDepth = 1000.0;
float MinDepth = 100;

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;

void main()
{
    vec4 p = viewMatrix * modelMatrix * gl_in[gl_InvocationID].gl_Position;
    // "Distance" from camera scaled between 0 and 1
    float depth = clamp( (abs(p.z) - MinDepth) / (MaxDepth - MinDepth),0.0, 1.0 );
    // Interpolate between min/max tess levels
    float tessLevel = mix(MaxTessLevel, MinTessLevel, depth);
    gl_TessLevelOuter[0] = float(tessLevel);
    gl_TessLevelOuter[1] = float(tessLevel);
    gl_TessLevelOuter[2] = float(tessLevel);
    gl_TessLevelOuter[3] = float(tessLevel);
    gl_TessLevelInner[0] = float(tessLevel);
    gl_TessLevelInner[1] = float(tessLevel);
    gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;

}

