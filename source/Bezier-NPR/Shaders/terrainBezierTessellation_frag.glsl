#version 410 core

// output shader Color
out vec4 out_Color;

// Inputs from Tessellation Evaluation Shader
in float normalDotViewValue;

// Structure to pass data to Tessellation Evaluation Shader
in CURVATURE_INFO{
    vec2 uvCoordinatesInBezierPatch;
    mat2 firstFundamentalFormMatrix;
    mat2 secondFundamentalFormMatrix;
    float k1;
    float k2;
    float meanCurvature;
    float gaussianCurvature;
    vec3 principalDirection1;
    vec3 principalDirection2;
    vec3 viewVectorProjectedInTangentPlane;
    mat3 TBN;
    vec2 w;
    float normalCurvatureInDirectionW;
} curvature_informations;

// Normal in view coordinates
in vec3 viewNormal;
// Light direction in view coordinates
in vec3 viewLightDirection;
// Vector to Camera in view coordinate
in vec3 vectorToCamera;

// Uniforms from user
uniform float contourLimit;
uniform float directionalDerivativeLimit;
// Colors to achieve desired style
uniform vec3 warmColor;
uniform vec3 coldColor;
uniform vec3 strokeColor;
// Numbers of levels for cel Shading
uniform int celShadingSize;
uniform int shininessFactor;
// settings from UI
uniform int shadingType;
uniform bool enableContours;
uniform bool enableSuggestiveContours;


////////////////////////////////////////////////////////////////////
// calculate light value at the current fragment
float lightIntensity() 
{
  vec3 N = normalize(viewNormal);
  vec3 L = normalize(viewLightDirection.xyz);
  float lambertian = max(dot(L,N), 0.0);
  vec3 V = normalize( vectorToCamera );
  // Uses half vector
  vec3 H = normalize(L + V);
  float specAngle = max(dot(H, N), 0.0);
  float specular = pow(specAngle, shininessFactor);
  float ambientWeight  = 0.2f;
  float diffuseWeight  = 0.9f * lambertian;
  float SpecularWeight = 0.1f * specular;
  // Return an estimation of current light value of the fragment
  return clamp(  diffuseWeight + SpecularWeight, ambientWeight, 1 );
}

//Used for celShading to extract a level of light from a value
float GetLevelFromValue( float value, int levels )  
{
    int app = int( value * 100 );
    return ( app - ( app % levels ) ) / 100.f ;
}


vec3 CelShading()
{
  float curretFragLight = lightIntensity();
  float fragLightAfterLevelSuddivision = GetLevelFromValue( curretFragLight, celShadingSize );
  vec3 N = normalize(viewNormal);
  vec3 L = normalize(viewLightDirection.xyz);
  float DiffuseFactor = dot(L, N);
  DiffuseFactor = floor(DiffuseFactor * celShadingSize) * (1.0f/celShadingSize);
  return mix( coldColor, warmColor, fragLightAfterLevelSuddivision );
}

vec3 GoochShading(){
  // normalization of the per-fragment light incidence direction
  vec3 L = normalize(viewLightDirection.xyz);
  // normalization of the per-fragment normal
  vec3 N = normalize(viewNormal);
  // Lambert coefficient
  float lambertian = dot(L, N);
  // weight used in standard gooch shading
  float weight = ( lambertian + 1.0 ) * 0.5;
  // Diffuse component of standard gooch shading
  vec3 kCool = min(coldColor + 0.25 * vec3(0.65, 0.65, 0.65), 1.0);
  vec3 kWarm = min(warmColor + 0.5 * vec3(0.65, 0.65, 0.65), 1.0); 
  vec3 kFinal = mix(kCool, kWarm, weight);
  // Calculation of specular component
  vec3 R = normalize(reflect(-L, N));
  vec3 V = normalize( vectorToCamera );
  float specAngle = max(dot(R, V), 0.0);
  // shininess application to the specular component
  float specular = pow(specAngle, shininessFactor);
  // Final color composition of the standard gooch shading
  return vec3(kFinal + vec3(1) * specular);
}

vec3 Contours()
{
  vec3 color = vec3(1.0, 1.0, 1.0);
  float cLimitCalculated = (pow(normalDotViewValue, 2.0));
  float dd = directionalDerivativeLimit * 0.0001;
  // Derivate of normal Curvature in direction W, DwKr
  // It is approximated as composition of dFdx and dFdy
  // DwKr = w.x * dFdx + w.y * dFdy
  float derivateNormalCurvatureInDirectionW = 
      curvature_informations.viewVectorProjectedInTangentPlane.x * dFdx(curvature_informations.normalCurvatureInDirectionW) 
    + curvature_informations.viewVectorProjectedInTangentPlane.y * dFdy(curvature_informations.normalCurvatureInDirectionW);
  // Contours are those points where N dot V = 0
  // contourLimits is used to stretch the definition interval so that contours are those 
  // points where 0 <= N dot V <= contourLimits
  if(enableContours && cLimitCalculated<contourLimit)
    color = strokeColor;
  // Suggestive Contours are those points where Kr = 0 and DwKr > 0
  // directionalDerivateLimit (dd) is used to stretch the definition interval so
  // that Suggestive Contours are those points where -dd <= Kr <= dd && DwKr > 0
  else if( enableSuggestiveContours 
    && curvature_informations.normalCurvatureInDirectionW >= -dd 
    && curvature_informations.normalCurvatureInDirectionW < dd 
    && derivateNormalCurvatureInDirectionW>0 ){
      color = mix(vec3(1.0), strokeColor, 0.75);
  }
  return color;
}

//////////////////////////////////////////
// main
void main(void)
{   
    vec3 color;

    if (shadingType == 0){
        color = CelShading();
    }
    else if ( shadingType == 1){
        color = GoochShading();
    }
    else{
      color = warmColor;
    }

    if (enableContours || enableSuggestiveContours){
          color *= Contours();
    }

    //Final Fragment Color
    out_Color = vec4(color, 1.0);
}
