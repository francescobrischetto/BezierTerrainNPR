#version 410 core
// Object Space Suggestive Contours vertex shader
// Jeroen Baert - www.forceflow.be

// IN: computed values from vertex shader
	in float ndotv;
	in float t_kr;
	in float t_dwkr;
	in float fz;

// IN: uniform values
	uniform float c_limit;
	uniform float sc_limit;
	uniform float dwkr_limit;

    out vec4 out_Color;

////////////////////////////////////////////////////////////////////

// the "type" of the Subroutine
subroutine vec3 ill_model();
// Subroutine Uniform (it is conceptually similar to a C pointer function)
subroutine uniform ill_model Illumination_Model;

////////////////////////////////////////////////////////////////////

///////////////////ILLUMINATION MODELS///////////////////////
// a subroutine for the Lambert model
subroutine(ill_model)
vec3 SuggestiveCountorns() // this name is the one which is detected by the SetupShaders() function in the main application, and the one used to swap subroutines
{  
	// base color
	vec3 color = vec3(1.0f, 1.0f, 1.0f); 
	
	// use feature size
	float kr = fz*abs(t_kr); // absolute value to use it in limits
	float dwkr = fz*fz*t_dwkr; // two times fz because derivative
	float dwkr2 = (dwkr-dwkr*pow(ndotv, 2.0));

	// compute limits
	float contour_limit = c_limit*(pow(ndotv, 2.0)/kr);
	float sc_limit = sc_limit*(kr/dwkr2);
	// contours
	if(contour_limit<1.0)
	{color.xyz = min(color.xyz, vec3(contour_limit, contour_limit, contour_limit));}
	// suggestive contours
	else if((sc_limit<1.0) && dwkr2>dwkr_limit)
	{color.xyz = min(color.xyz, vec3(sc_limit, sc_limit, sc_limit));}
	return color;
}
//////////////////////////////////////////


//////////////////////////////////////////
// main
void main(void)
{
  	vec3 color = Illumination_Model(); 
    out_Color = vec4(color, 1.0);
}

