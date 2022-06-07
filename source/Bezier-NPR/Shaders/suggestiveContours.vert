#version 410 core
// Object Space Suggestive Contours vertex shader
// Jeroen Baert - www.forceflow.be

// vertex position in world coordinates
layout (location = 0) in vec3 position;
// vertex normal in world coordinate
layout (location = 1) in vec3 normal;
// Trimesh useful properties //TODO FIX
layout (location = 2) in float curv1;
layout (location = 3) in float curv2;
layout (location = 4) in float feature_size;
layout (location = 5) in vec3 pdir1;
layout (location = 6) in vec3 pdir2;
layout (location = 7) in vec4 dcurv;


// IN: camera position (per render) 
uniform vec3 gEyeWorldPos;
// model matrix
uniform mat4 modelMatrix;
// view matrix
uniform mat4 viewMatrix;
// Projection matrix
uniform mat4 projectionMatrix;
// Projection matrix
uniform mat3 normalMatrix;


// OUT: variables for fragment shader
out float ndotv;
out float t_kr;
out float t_dwkr;
out float fz;

void main()
{
	fz = feature_size;
	// compute vector to cam
	vec4 mvPosition = viewMatrix * modelMatrix * vec4( position, 1.0 );
	vec3 view = normalize(-mvPosition.xyz);
	vec3 vNormal = normalize( normalMatrix * normal );
	// compute ndotv (and normalize view)
	//ndotv = (1.0f / length(view)) * dot(vNormal,view);
	ndotv = max(dot(vNormal,view), 0.0);
	// optimalisation: if this vector points away from cam, don't even bother computing the rest.
	// the data will not be used in computing pixel color
	if(!(ndotv < 0.0f))
	{
		// compute kr
		vec3 w = normalize(view - vNormal * dot(view, vNormal));
  		float u = dot(w, pdir1);
  		float v = dot(w, pdir2);
  		float u2 = u*u;
    	float v2 = v*v;
  		t_kr = (curv1*u2) + (curv2*v2);
  		// and dwkr
  		float uv = u*v;
  		float dwII = (u2*u*dcurv.x) + (3.0*u*uv*dcurv.y) + (3.0*uv*v*dcurv.z) + (v*v2*dcurv.w);
  		// extra term due to second derivative
  		t_dwkr = dwII + 2.0 * curv1 * curv2 * ndotv/sqrt((1.0 - pow(ndotv, 2.0)));
  	}
  	
	// position transformation
	//gl_Position = ftransform();
    gl_Position = projectionMatrix * mvPosition;
} 
