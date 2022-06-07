/*

02_flatten.frag : Fragment shader, equal to 01_fullcolor.frag

author: Davide Gadia

Real-Time Graphics Programming - a.a. 2020/2021
Master degree in Computer Science
Universita' degli Studi di Milano

*/

#version 410 core

// output shader variable
out vec4 colorFrag;

////////////////////////////////////////////////////////////////////

// the "type" of the Subroutine
subroutine vec3 ill_model();
// Subroutine Uniform (it is conceptually similar to a C pointer function)
subroutine uniform ill_model Illumination_Model;

////////////////////////////////////////////////////////////////////

///////////////////ILLUMINATION MODELS///////////////////////
// a subroutine for the Lambert model
subroutine(ill_model)
vec3 Lambert() // this name is the one which is detected by the SetupShaders() function in the main application, and the one used to swap subroutines
{  
    return vec3(1.0,0.0,0.0);
}
//////////////////////////////////////////


//////////////////////////////////////////
// main
void main(void)
{
  	vec3 color = Illumination_Model(); 
    colorFrag = vec4(color, 1.0);
}