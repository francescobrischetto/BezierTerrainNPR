/*
Project of Francesco Brischetto: This project is based based on "Geometry-based shading for shape depiction enhancement".
                                 It applies an NPR effect to the illumination model that enhances object shape 
                                 based on object local geometry

illumination_models_modified_fr.frag: Fragment shader for the Lambert, Blinn-Phong, Gooch and Cartoon illumination models
It receives smoothed surface normal, computed in model loading phase, that will be used to calculate the sharpened surface

N.B. 1)  "illumination_models_modified_vt.vert" must be used as vertex shader

N.B. 2)  the different illumination models are implemented using Shaders Subroutines

author: Davide Gadia
refined by: Francesco Brischetto  mat. 958022

Real-Time Graphics Programming - a.a. 2020/2021
Master degree in Computer Science
Universita' degli Studi di Milano

*/

#version 410 core

// output shader variable
out vec4 colorFrag;

// light incidence direction (calculated in vertex shader, interpolated by rasterization)
in vec3 lightDir;
// the transformed normal has been calculated per-vertex in the vertex shader
in vec3 vNormal;
// the transformed smoothed normal has been calculated per-vertex in the vertex shader
in vec3 vSMNormal;
// vector from fragment to camera (in view coordinate)
in vec3 vViewPosition;

// uniforms for Blinn-Phong model
// ambient, diffusive and specular components (passed from the application)
uniform vec3 ambientColor;
uniform vec3 diffuseColor;
uniform vec3 specularColor;
// weight of the components
// in this case, we can pass separate values from the main application even if Ka+Kd+Ks>1. In more "realistic" situations, I have to set this sum = 1, or at least Kd+Ks = 1, by passing Kd as uniform, and then setting Ks = 1.0-Kd
uniform float Ka;
uniform float Kd;
uniform float Ks;
// shininess coefficients (passed from the application)
uniform float shininess;

// uniforms used as control parameters for all the functions defined in the reference paper
// Equation 8 chapter 5.1 and Equation 6 chapter 4.2.2 reference paper
uniform float lambda;
// Equation 8 chapter 5.1 reference paper
uniform float alpha;
// Equation 13 chapter 6.2 reference paper
uniform float r;
// Equation 13 chapter 6.2 reference paper
uniform float Ql;

// uniforms defined by me to compose the three components in final result of Ehnaced Toon Shading and Enhanced Gooch Shading. 
// This was not specified in the reference paper
uniform float myWeightA;
uniform float myWeightD;

// uniforms for Toon Shading Model
uniform vec3 shinestColor;
uniform vec3 shinyColor;
uniform vec3 darkColor;
uniform vec3 gloomyColor;

// uniforms for Gooch Shading model
uniform vec3  SurfaceColor;
uniform vec3  WarmColor;
uniform vec3  CoolColor;
uniform float DiffuseWarm;
uniform float DiffuseCool;

// uniforms for depth linearization
uniform float near;
uniform float far;

////////////////////////////////////////////////////////////////////

// the "type" of the Subroutine
subroutine vec3 ill_model();
// Subroutine Uniform (it is conceptually similar to a C pointer function)
subroutine uniform ill_model Illumination_Model;

////////////////////////////////////////////////////////////////////

///////////////////HELPING FUNCTIONS///////////////////////
// Function to linearize the depth values to use depth. 
// It can be found here: https://learnopengl.com/Advanced-OpenGL/Depth-testing
float LinearizeDepth(float depth) 
{
    float z = depth * 2.0 - 1.0; // back to NDC 
    return (2.0 * near * far) / (far + near - z * (far - near));	
}
//////////////////////////////////////////
// Curvature-Based Reflectance Scaling Function (used by Enhanced Blinn-Phong model)
// Defined in the reference paper in chapter 5.1
float Lr(float curvature_value, float delta)
{
  // We apply the curvature mapping function that uses lambda and alpha parameters to apply non-linear mapping
  float P = pow (lambda * abs(curvature_value), alpha);
  // Uses as intensity mapping function the second parameter, delta
  // This function maps intensity mapping and curvature mapping functions in the reflectance radiance equation
  // This aims to correlate the reflected lightning intensity to surface curvature
  float G = delta / ( exp(P) * ( 1 - delta ) + delta );
  return G;
}

//////////////////////////////////////////
// My proposed method for calculating curvature (used by Enhanced Blinn-Phong model)
// Based on: https://madebyevan.com/shaders/curvature/
float curvature(vec3 N_I)
{
  // We compute curvature exploiting partial derivatives of the Enhanced Surface Normal
  vec3 dx = dFdx(N_I);
  vec3 dy = dFdy(N_I);
  float depth = LinearizeDepth(gl_FragCoord.z);
  float curvature_value = (cross(N_I - dx, N_I + dx).y - cross(N_I - dy, N_I + dy).x) * 4.0 / depth;
  return curvature_value;
  //  return clamp(curvature_value, -1, 1);
}
//////////////////////////////////////////

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
    return vec3(Kd * lambertian * diffuseColor);
}
//////////////////////////////////////////

//////////////////////////////////////////
// a subroutine to visualize Enhanced Normal vector
subroutine(ill_model)
vec3 VisualizeEnhancedNormal()
{
    // Computing the mask for Unsharp Masking
    vec3 mask = vNormal - vSMNormal;
    // calculating enhanced Normal using the Unsharp Masking technique
    // This is defined, in the reference paper, in equation 6 of chapter 4.2.2
    vec3 eNormal = vNormal + lambda * mask;
    // normalization of the per-fragment enhanced normal 
    vec3 N_I = normalize(eNormal);
    return N_I;
}
//////////////////////////////////////////

//////////////////////////////////////////
// a subroutine to visualize Normal vector
subroutine(ill_model)
vec3 VisualizeNormal()
{
    // normalization of the per-fragment normal
    vec3 N = normalize(vNormal);
    return N;
}
//////////////////////////////////////////

//////////////////////////////////////////
// a subroutine to visualize Enhanced Curvature
subroutine(ill_model)
vec3 VisualizeEnhancedCurvature()
{
    // Computing the mask for Unsharp Masking
    vec3 mask = vNormal - vSMNormal;
    // calculating enhanced Normal using the Unsharp Masking technique
    // This is defined, in the reference paper, in equation 6 of chapter 4.2.2
    vec3 eNormal = vNormal + lambda * mask;
    // normalization of the per-fragment enhanced normal 
    vec3 N_I = normalize(eNormal);
    // calculating curvature value using enhanced normal
    float curvature_value = curvature(N_I);
    vec3 ambient = vec3(curvature_value + 0.5);
    return ambient;
    
}
//////////////////////////////////////////

//////////////////////////////////////////
// a subroutine to visualize Curvature
subroutine(ill_model)
vec3 VisualizeCurvature()
{
    // normalization of the per-fragment normal
    vec3 N = normalize(vNormal);
    // calculating curvature value using normal vector
    float curvature_value = curvature(N);
    vec3 ambient = vec3(curvature_value + 0.5);
    return ambient;
    
}
//////////////////////////////////////////

//////////////////////////////////////////
// a subroutine for the Gooch Shading model using Shape Depiction Enhancement based on local Geometry 
subroutine(ill_model)
vec3 EnhancedGoochShading(){
  // normalization of the per-fragment light incidence direction
  vec3 L = normalize(lightDir.xyz);
  // Computing the mask for Unsharp Masking
  vec3 mask = vNormal - vSMNormal;
  // calculating enhanced Normal using the Unsharp Masking technique
  // This is defined, in the reference paper, in equation 6 of chapter 4.2.2
  vec3 eNormal = vNormal + lambda * mask;
  // normalization of the per-fragment enhanced normal 
  vec3 N_I = normalize(eNormal);
  // NOTE: Reference paper use the costant 1 as rho_a component for ambient
  float rhoA = 1;
  // Equation 14 of the Chapter 6.3 of the reference paper applied only to diffuse and ambient components 
  float deltaA = (1 + rhoA) * 0.5;
  // NOTE: Reference paper use the lambertian coefficient as rho_d for diffuse
  float rhoD = max(dot(L,N_I), 0.0);
  // Equation 14 of the Chapter 6.3 of the reference paper applied only to diffuse and ambient components 
  float deltaD = (1 + rhoD) * 0.5;
  // Calculation of specular component, specified as in the reference paper, using the same as Phong model
  vec3 R = normalize(reflect(-L, N_I));
  vec3 V = normalize( vViewPosition );
  float specAngle = max(dot(R, V), 0.0);
  // shininess application to the specular component
  float rhoS = pow(specAngle, shininess);
  // Diffuse component of standard gooch shading 
  //but using our previously calculated delta as weight, for diffuse and ambient component
  vec3 kCool = min(CoolColor + DiffuseCool * SurfaceColor, 1.0);
  vec3 kWarm = min(WarmColor + DiffuseWarm * SurfaceColor, 1.0);
  vec3 kFinalA = mix(kCool,kWarm, deltaA);
  vec3 kFinalD = mix(kCool,kWarm, deltaD);
  // Composition of the final color.
  // NOTE: In the paper is not specified how the three components are composed.
  //       This is my solution that considers only Ambient and Diffuse components, while maintaning full specular component
  return  myWeightA * kFinalA + myWeightD * kFinalD + vec3(1) * rhoS;
}
//////////////////////////////////////////

//////////////////////////////////////////
// a subroutine for the Gooch Shading model
subroutine(ill_model)
vec3 GoochShading(){
  // normalization of the per-fragment light incidence direction
  vec3 L = normalize(lightDir.xyz);
  // normalization of the per-fragment normal
  vec3 N = normalize(vNormal);
  // Lambert coefficient
  float lambertian = dot(L, N);
  // weight used in standard gooch shading
  float weight = ( lambertian + 1.0 ) * 0.5;
  // Diffuse component of standard gooch shading
  vec3 kCool = min(CoolColor + DiffuseCool * SurfaceColor, 1.0);
  vec3 kWarm = min(WarmColor + DiffuseWarm * SurfaceColor, 1.0); 
  vec3 kFinal = mix(kCool, kWarm, weight);
  // Calculation of specular component
  vec3 R = normalize(reflect(-L, N));
  vec3 V = normalize( vViewPosition );
  float specAngle = max(dot(R, V), 0.0);
  // shininess application to the specular component
  float specular = pow(specAngle, shininess);
  // Final color composition of the standard gooch shading
  return vec3(kFinal + vec3(1) * specular);
}
//////////////////////////////////////////

//////////////////////////////////////////
// a subroutine for the Enhanced Cartoon/Cel Shading model using Shape Depiction Enhancement based on local Geometry 
subroutine(ill_model)
vec3 EnhancedToonShading(){
  // normalization of the per-fragment light incidence direction
  vec3 L = normalize(lightDir.xyz);
  // Computing the mask for Unsharp Masking
  vec3 mask = vNormal - vSMNormal;
  // calculating enhanced Normal using the Unsharp Masking technique
  // This is defined, in the reference paper, in equation 6 of chapter 4.2.2
  vec3 eNormal = vNormal + lambda * mask;
  // normalization of the per-fragment enhanced normal 
  vec3 N_I = normalize(eNormal);
  // NOTE: Reference paper use the costant 1 as rho_a component for ambient
  float rhoA = 1;
  // Intensity parameter used in standard toon/cel shading, but using our enhanced normal, for the ambient compinent
  // Equation 13 of the Chapter 6.2 of the reference paper applied only to diffuse and ambient components 
  float deltaA = floor(0.5 + (Ql * pow(rhoA, r))) / Ql;
  // NOTE: Reference paper use the lambertian coefficient as rho_d for diffuse
	float rhoD = max(dot(L,N_I), 0.0);
  // Intensity parameter used in standard toon/cel shading, but using our enhanced normal, for the diffuse component
  // Equation 13 of the Chapter 6.2 of the reference paper applied only to diffuse and ambient components 
  float deltaD = floor(0.5 + (Ql * pow(rhoD, r))) / Ql;
  // Calculation of specular component, specified as in the reference paper, using the same as Phong model
  vec3 R = normalize(reflect(-L, N_I));
  vec3 V = normalize( vViewPosition );
  float specAngle = max(dot(R, V), 0.0);
  // shininess application to the specular component
  // NOTE: Reference paper use the lambertian coefficient as rho_s for specular
  float deltaS = pow(specAngle, shininess);
  // Composition of the final intensity to apply, then, the color choice.
  // NOTE: In the paper is not specified how the three components are composed.
  //       This is my solution that considers only Ambient and Diffuse components, while maintaning full specular component
  float intensity = myWeightA * deltaA + myWeightD * deltaD + deltaS;
  // Color choice based on intensity parameter
	if (intensity > 0.95)       return shinestColor;
	else if (intensity > 0.5)   return shinyColor;
	else if (intensity > 0.25)  return darkColor;
	else                        return gloomyColor;
}
//////////////////////////////////////////

//////////////////////////////////////////
// a subroutine for the Cartoon/Cel Shading model
subroutine(ill_model)
vec3 ToonShading(){
  // normalization of the per-fragment light incidence direction
  vec3 L = normalize(lightDir.xyz);
  // normalization of the per-fragment normal
  vec3 N = normalize(vNormal);
  // Intensity parameter used in standard toon/cel shading
	float intensity = dot(L,N);
  // Color choice based on intensity parameter
	if (intensity > 0.95)       return shinestColor;
	else if (intensity > 0.5)   return shinyColor;
	else if (intensity > 0.25)  return darkColor;
	else                        return gloomyColor;
}
//////////////////////////////////////////

//////////////////////////////////////////
// a subroutine for the Enhanced Blinn-Phong model using Shape Depiction Enhancement based on local Geometry 
subroutine(ill_model)
vec3 EnhancedBlinnPhong()
{
  // Computing the mask for Unsharp Masking
  vec3 mask = vNormal - vSMNormal;
  // calculating enhanced Normal using the Unsharp Masking technique
  // This is defined, in the reference paper, in equation 6 of chapter 4.2.2
  vec3 eNormal = vNormal + lambda * mask;
  // normalization of the per-fragment enhanced normal 
  vec3 N_I = normalize(eNormal);
  // calculating curvature value using enhanced normal
  float curvature_value = curvature(N_I);
  // Implementing equation 12 of chapter 6.1 of the reference paper
  // I calculate the Curvature-Based Reflectance Scaling factor for each of the Blinn-Phong components
  // NOTE: Reference paper use the costant 1 as rho_a component for ambient
  float rhoA = 1;
  float G_a = Lr(curvature_value, rhoA);  
  // ambient component can be calculated at this point
  vec3 color = Ka * G_a * ambientColor;
  // normalization of the per-fragment light incidence direction
  vec3 L = normalize(lightDir.xyz);
  // Lambert coefficient
  float rhoD = max(dot(L,N_I), 0.0);
  // if the lambert coefficient is positive, then I can calculate the specular component
  if(rhoD > 0.0)
  {
    // This is the Curvature-Based Reflectance Scaling factor for the diffuse component
    // NOTE: Reference paper use the lambertian coefficient as rho_d for diffuse
    float G_d = Lr(curvature_value, rhoD); 
    // the view vector has been calculated in the vertex shader, already negated to have direction from the mesh to the camera
    vec3 V = normalize( vViewPosition );
    // in the Blinn-Phong model we do not use the reflection vector, but the half vector
    vec3 H = normalize(L + V);
    // we use H to calculate the specular component
    float specAngle = max(dot(H, N_I), 0.0);
    // shininess application to the specular component
    float rhoS = pow(specAngle, shininess);
    // This is the Curvature-Based Reflectance Scaling factor for the specular component
    // NOTE: Reference paper use the lambertian coefficient as rho_s for specular
    float G_s = Lr(curvature_value, rhoS);
    // We add diffusive and specular components to the final color using our Curvature-Based factors
    color += vec3( Kd * G_d * diffuseColor +
                   Ks * G_s * specularColor);
  }
  return color;
}
//////////////////////////////////////////

//////////////////////////////////////////
// a subroutine for the Blinn-Phong model
subroutine(ill_model)
vec3 BlinnPhong() // this name is the one which is detected by the SetupShaders() function in the main application, and the one used to swap subroutines
{
    // ambient component can be calculated at the beginning
    vec3 color = Ka*ambientColor;
    // normalization of the per-fragment normal
    vec3 N = normalize(vNormal);
    // normalization of the per-fragment light incidence direction
    vec3 L = normalize(lightDir.xyz);
    // Lambert coefficient
    float lambertian = max(dot(L,N), 0.0);
    // if the lambert coefficient is positive, then I can calculate the specular component
    if(lambertian > 0.0)
    {
      // the view vector has been calculated in the vertex shader, already negated to have direction from the mesh to the camera
      vec3 V = normalize( vViewPosition );
      // in the Blinn-Phong model we do not use the reflection vector, but the half vector
      vec3 H = normalize(L + V);
      // we use H to calculate the specular component
      float specAngle = max(dot(H, N), 0.0);
      // shininess application to the specular component
      float specular = pow(specAngle, shininess);
      // We add diffusive and specular components to the final color
      // N.B. ): in this implementation, the sum of the components can be different than 1
      color += vec3( Kd * lambertian * diffuseColor +
                     Ks * specular * specularColor);
    }
    return color;
}
//////////////////////////////////////////

//////////////////////////////////////////
// main
void main(void)
{
    // we call the pointer function Illumination_Model():
    // the subroutine selected in the main application will be called and executed
  	vec3 color = Illumination_Model(); 
    colorFrag = vec4(color, 1.0);
}
