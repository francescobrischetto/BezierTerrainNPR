#version 410 core
// output shader variable
out vec4 colorFrag;


in vec3 vNormal;
in vec3 lightDir;

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
    // normalization of the per-fragment normal
    vec3 N = normalize(vNormal);
    // normalization of the per-fragment light incidence direction
    vec3 L = normalize(lightDir.xyz);
    // Lambert coefficient
    float lambertian = max(dot(L,N), 0.0);
    // Lambert illumination model  
    return vec3(lambertian * vec3(1.0,0.0,0.0));
}
//////////////////////////////////////////
// a subroutine for the Lambert model
subroutine(ill_model)
vec3 Normal() // this name is the one which is detected by the SetupShaders() function in the main application, and the one used to swap subroutines
{
    // normalization of the per-fragment normal
    vec3 N = normalize(vNormal);
    return vec3(N);
}
//////////////////////////////////////////


//////////////////////////////////////////
// main
void main(void)
{
  	vec3 color = Illumination_Model(); 
    colorFrag = vec4(color, 1.0);
}
