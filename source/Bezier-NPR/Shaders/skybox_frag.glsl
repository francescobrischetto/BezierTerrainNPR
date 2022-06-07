/*
18_skybox.frag: fragment shader for the visualization of the cube map as environment map

author: Davide Gadia

Real-Time Graphics Programming - a.a. 2020/2021
Master degree in Computer Science
Universita' degli Studi di Milano
*/

/*
OpenGL coordinate system (right-handed)
positive X axis points right
positive Y axis points up
positive Z axis points "outside" the screen


                              Y
                              |
                              |
                              |________X
                             /
                            /
                           /
                          Z
*/

#version 410 core

// output shader variable
out vec4 colorFrag;

// interpolated texture coordinates
in vec3 interp_UVW;

// texture sampler for the cube map
uniform samplerCube skyboxCube;
uniform vec3 backgroundColor;

void main()
{
	 // we sample the cube map
    colorFrag = texture(skyboxCube, interp_UVW) * vec4(backgroundColor,1.0);
}
