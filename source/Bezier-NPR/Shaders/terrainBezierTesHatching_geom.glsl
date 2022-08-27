#version 410 core

layout(points) in;
layout(line_strip, max_vertices = 2) out;

in vec3 pdir1[];
in vec3 pdir2[];
in vec3 LNormal[];
in float ndotv[];

void main()
{
    gl_Position = gl_in[0].gl_Position;
    EmitVertex();

    gl_Position = gl_in[0].gl_Position + vec4(LNormal[0],1.0) * 128 ;
    EmitVertex();

    EndPrimitive();

}