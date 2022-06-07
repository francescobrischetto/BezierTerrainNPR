#version 410 core

// We want to generate triangles with an equal spacing pattern
layout(triangles,equal_spacing) in;

// Structure to take data from Tessellation Control Shader
in TCS_OUT{
    vec3 position;
    vec3 normal;
    float curv1;
    float curv2;
    float feature_size;
    vec3 pdir1;
    vec3 pdir2;
    vec4 dcurv;
} tes_in [];

// Uniforms
// View Matrix
uniform mat4 viewMatrix;
// Projection matrix
uniform mat4 projectionMatrix;
// Normal Matrix
uniform mat3 normalMatrix;
// model matrix
uniform mat4 modelMatrix;
// Point Light Position in world Space
uniform vec3 pointLightWorldPosition;

// Output parameters to the fragment shader
out vec4 fragPos;
out vec3 fragNorm;
// Normal in view coordinates
out vec3 viewNormal;
// Light direction in view coordinates
out vec3 viewLightDirection;
// Dot product between Normal and View vector in view coordinates
out float dotNV;
// Curvature
out float curvatureValue;
// Curvature Derivate
out float curvatureDerivate;
// Mesh feature size -> median edge length
out float feature_size;
// Vector to Camera in view coordinate
out vec3 vecToCamera;

// Functions to interpolate values between the three generated points
vec3 lerp3D(vec3 v0, vec3 v1, vec3 v2)
{
    return vec3(gl_TessCoord.x) * v0 + vec3(gl_TessCoord.y) * v1 + vec3(gl_TessCoord.z) * v2;
}

float lerp3Dfloat(float f0, float f1, float f2)
{
    return (f0 + f1 + f2)/3;
}

vec4 lerp4D(vec4 v0, vec4 v1, vec4 v2)
{
    return vec4(gl_TessCoord.x) * v0 + vec4(gl_TessCoord.y) * v1 + vec4(gl_TessCoord.z) * v2; 
}

void main(){
    // We determine all the values by interpolating between three generated points
    vec3 position   = lerp3D(tes_in[0].position,tes_in[1].position,tes_in[2].position);
    vec3 normal     = lerp3D(tes_in[0].normal, tes_in[1].normal, tes_in[2].normal);
    vec3 pdir1      = lerp3D(tes_in[0].pdir1, tes_in[1].pdir1, tes_in[2].pdir1);
    vec3 pdir2      = lerp3D(tes_in[0].pdir2, tes_in[1].pdir2, tes_in[2].pdir2);
    float curv1     = lerp3Dfloat(tes_in[0].curv1, tes_in[1].curv1, tes_in[2].curv1);
    float curv2     = lerp3Dfloat(tes_in[0].curv2, tes_in[1].curv2, tes_in[2].curv2);
    feature_size    = lerp3Dfloat(tes_in[0].feature_size, tes_in[1].feature_size, tes_in[2].feature_size);
    vec4 dcurv      = lerp4D(tes_in[0].dcurv, tes_in[1].dcurv, tes_in[2].dcurv);
    
    
    fragPos = vec4(position, 1.0);
    fragNorm =  ( modelMatrix * vec4( normal, 0.0 ) ).xyz;
	// Position in view coordinates
	vec4 mvPosition = viewMatrix * vec4( position, 1.0 );
    // Vector to camera
    vecToCamera = -mvPosition.xyz;
    // Versor to camera
	vec3 viewVersor = normalize(vecToCamera);
    // Normal in view coordinates
	viewNormal = normalize(normalMatrix * normal);
    // Dot product between Normal and View vector in view coordinates
	dotNV = dot(viewNormal,viewVersor);

    // Following computations are done for contours and suggestive contours

	// If this vector points away from camera, don't compute the rest
	if( !( dotNV < 0.0f ) )
	{   
		// Compute curvature using Principal Directions and curvatures values along those directions
		vec3 w  = normalize( viewVersor - viewNormal * dotNV );
  		float u = dot( w, pdir1 );
  		float v = dot( w, pdir2 );
  		float uSquared = u*u;
    	float vSquared = v*v;
  		curvatureValue = ( curv1 * uSquared ) + ( curv2 * vSquared );
  		// Compute curvature derivative using Directional derivatives in Principal Curvature Directions
  		float uv = u*v;
        // We consider also second derivative 
  		float curvSecondTerm = ( uSquared * u * dcurv.x ) 
                             + ( 3.0 * u * uv * dcurv.y ) 
                             + ( 3.0 * uv * v * dcurv.z ) 
                             + ( v * vSquared * dcurv.w );
  		curvatureDerivate = curvSecondTerm + 2.0 * curv1 * curv2 * dotNV/sqrt((1.0 - pow(dotNV, 2.0)));
  	}
    // Light position in view coordinates
    vec4 lightPos = viewMatrix  * vec4(pointLightWorldPosition, 1.0);
    // Light vector in view coordinates
    viewLightDirection = lightPos.xyz - mvPosition.xyz;
    // Final position after projection
    gl_Position = projectionMatrix * mvPosition;



}