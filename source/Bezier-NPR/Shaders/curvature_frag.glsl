#version 410 core

in vec3 geometry_k1_k2;
in vec4 geometry_min_dir;
in vec4 geometry_max_dir;
in vec4 geometry_position;
in vec4 geometry_normale;

float u_kmin = -2.0;
float u_kmax = 2.0;

out vec4 fragment_color;

// model matrix
uniform mat4 modelMatrix;
// view matrix
uniform mat4 viewMatrix;
// Projection matrix
uniform mat4 projectionMatrix;
uniform vec3 pointLightWorldPosition;

vec3 gradientmap( float scale )
{
  const int intervals = 2; //3 colors
  int upper_index = int( ceil( intervals * scale ) );
  if ( upper_index == 0 ) // Special case when value == min.
    upper_index = 1;
    
  vec3 colors[3];
  colors[0] = vec3(0, 0, 1);
  colors[1] = vec3(1, 0, 0);
  colors[2] = vec3(1, 1, 0);
  
  
  vec3 firstColor = colors[upper_index-1];
  vec3 lastColor = colors[upper_index];
	
  scale = ( scale * intervals ) - (upper_index - 1);

  return firstColor + scale * (lastColor - firstColor);
}

vec3 colorFromCurv(float c)
{
	vec3 color;
	float gt_curvature = c;
	if(u_kmax > u_kmin)
		gt_curvature = (c-u_kmin) / (u_kmax-u_kmin);

	if ((gt_curvature<0) || (gt_curvature>1)) color= vec3(0.5,0.5,0.5);
	else
		color= gradientmap(gt_curvature);
	
	return color;
}

void main( )
{
	vec3 color;
	color = colorFromCurv(geometry_k1_k2.x);

	
	vec3 normale;
	normale = geometry_normale.xyz;
	
	//Phong
	vec3 light_dir = vec3(1, 1, 1);
	normale = (viewMatrix * modelMatrix * vec4(normale, 0)).xyz;
	
	vec4 mvPosition = viewMatrix * modelMatrix * vec4( geometry_position, 1.0 );
	vec4 lightPos = viewMatrix  * vec4(pointLightWorldPosition, 1.0);
  	vec3 lightDir = lightPos.xyz - mvPosition.xyz;

	float shadow_weight = 0.5;
	float dotnormal = clamp(dot(normalize(normale), normalize(lightDir)), 0, 1);
	vec3 diffuse = shadow_weight * color * dotnormal + (1-shadow_weight) * color;
	
	vec3 R = reflect(-normalize(lightDir), normalize(normale));
	float ER = clamp(dot(normalize(vec3(0, 0, 1)), normalize(R)), 0, 1);
	vec3 specular = vec3(1) * pow(ER, 100);
	
	fragment_color = vec4(diffuse+specular, 1);
}