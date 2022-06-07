#version 410 core
// output shader Color
out vec4 out_Color;

// Inputs from Tessellation Evaluation Shader
in vec4 fragPos;
in vec3 fragNor;
// Normal in view coordinates
in vec3 viewNormal;
// Light direction in view coordinates
in vec3 viewLightDirection;
// Dot product between Normal and View vector in view coordinates  
in float dotNV;
// Curvature
in float curvatureValue;
// Curvature Derivate
in float curvatureDerivate;
// Mesh feature size -> median edge length
in float feature_size;
// Vector to Camera in view coordinate
in vec3 vecToCamera;

// Uniforms from user
uniform float contourLimit;
uniform float suggestivecLimit;
uniform float directionalDerivativeLimit;
// Colors to achieve desired style
uniform vec3 warmColor;
uniform vec3 coldColor;
uniform vec3 strokeColor;
// Numbers of levels for cel Shading
uniform int celShadingSize;
uniform int shininessFactor;
// Hatch Texture
uniform vec3 cameraWorldPosition;
uniform sampler2D hatchTexture;
uniform sampler2D grassTexture;
// settings from UI
uniform int shadingType;
uniform bool enableContours;
uniform bool enableSuggestiveContours;
uniform bool enableHatching;
uniform bool enableGrassTexture;


////////////////////////////////////////////////////////////////////
// calculate light value at the current fragment
float lightIntensity() 
{
    vec3 N = normalize(viewNormal);
    vec3 L = normalize(viewLightDirection.xyz);
    float lambertian = max(dot(L,N), 0.0);
    vec3 V = normalize( vecToCamera );
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

//
float GetLevelFromValue( float value, int levels )  
{
    int app = int( value * 100 );
    return ( app - ( app % levels ) ) / 100.f ;
}


vec3 CelShading( )
{
    float curretFragLight = lightIntensity();
	float fragLightAfterLevelSuddivision = GetLevelFromValue( curretFragLight, celShadingSize );
    vec3 N = normalize(viewNormal);
    vec3 L = normalize(viewLightDirection.xyz);
    float DiffuseFactor = dot(L, N);
    DiffuseFactor = floor(DiffuseFactor * celShadingSize) * (1.0f/celShadingSize);
    //return mix( coldColor, warmColor, DiffuseFactor );
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
  vec3 V = normalize( vecToCamera );
  float specAngle = max(dot(R, V), 0.0);
  // shininess application to the specular component
  float specular = pow(specAngle, shininessFactor);
  // Final color composition of the standard gooch shading
  return vec3(kFinal + vec3(1) * specular);
}

// invert the color of (0-1) value
float Invert( float value )
{
    return ( -value ) + 1;
}


// rotate a UV texel by the given amount (radians)
vec2 RotateUV( vec2 uv, float rotation )
{
    return vec2(
        cos( rotation ) * ( uv.x ) + sin( rotation ) * ( uv.y ),
        cos( rotation ) * ( uv.y ) - sin( rotation ) * ( uv.x )
    );
}

vec4 GetBlendedHatch( sampler2D baseTex, int overlays, vec2 uv, bool rotate )
{
    float shiftAmt = 0.29f;
    float sum = 0;
    for( int i = 0; i < overlays; i++ )
    {
        vec2 base_uv = rotate && ( i % 2 == 0 ) ? RotateUV( uv, 1.571f ) : uv;
        sum += Invert( texture2D( baseTex, base_uv + vec2( shiftAmt, 1.29 * shiftAmt ) ).r );
        shiftAmt += 0.29;
    }

    sum = clamp( sum, 0, 1 );
    return vec4( vec3( Invert( sum ) ), 1 );
}



// Dynamic solid texture implementation for blending hatch/stroke 
// textures at various distances
vec4 GetDynamicSolidColor( vec3 pos ) 
{
    float zcam = distance( fragPos.xyz, cameraWorldPosition.xyz );
    float s = log2( zcam ) - floor( log2( zcam ) );
    vec3 uv = ( 1.5 * pos / pow( 2.0, floor( log2( zcam ) ) ) );
    
    vec3 normal = normalize( fragNor.xyz );
    float tone = max( 0, 1 - lightIntensity() );

    vec2 uv1 = 2.0f * uv.xy;
    vec2 uv2 = 2 * uv1;
    vec2 uv3 = 4 * uv1;
    vec2 uv4 = 8 * uv1;
    
    float a1 = s / 2.0;
    float a2 = 0.5 - (s / 6.0);
    float a3 = (1.0 / 3.0) - (s / 6.0);
    float a4 = (1.0 / 6.0) - (s / 6.0);

    vec4 tex0 = texture( hatchTexture, uv1 ) * a1 + texture( hatchTexture, uv2 ) * a2 +
                texture( hatchTexture, uv3 ) * a3 + texture( hatchTexture, uv4 ) * a4;
    vec4 tex1 = GetBlendedHatch( hatchTexture, 3, uv1, false ) * a1 + GetBlendedHatch( hatchTexture, 3, uv2, false ) * a2 +
                GetBlendedHatch( hatchTexture, 3, uv3, false ) * a3 + GetBlendedHatch( hatchTexture, 3, uv4, false ) * a4;
    vec4 tex2 = GetBlendedHatch( hatchTexture, 4, uv1, false ) * a1 + GetBlendedHatch( hatchTexture, 4, uv2, false ) * a2 +
            GetBlendedHatch( hatchTexture, 4, uv3, false ) * a3 + GetBlendedHatch( hatchTexture, 4, uv4, false ) * a4;
    vec4 tex3 = GetBlendedHatch( hatchTexture, 5, uv1, true ) * a1 + GetBlendedHatch( hatchTexture, 5, uv2, true ) * a2 +
            GetBlendedHatch( hatchTexture, 5, uv3, true ) * a3 + GetBlendedHatch( hatchTexture, 5, uv4, true ) * a4;

    vec4 toneCol = vec4( 1 );
    if ( tone <= 0.33 && tone >= 0 ) 
    {
        toneCol = mix( tex0, tex1, tone * 3 );
    }
    else if( tone <= 0.66 && tone > 0.33 )
    {
        toneCol = mix( tex1, tex2, ( tone - 0.33 ) * 3 );
    }
    else
    {
        toneCol = mix( tex2, tex3, ( tone - 0.66 ) * 3 );
    }

    // filter binary to force discrete stroke value
    return toneCol;
}

vec3 DynamicSolidHatch()
{
    vec3 normal = normalize( fragNor );
    vec3 strokeCol = GetDynamicSolidColor( vec3( fragPos.xz, fragPos.y ) ).rgb;
    strokeCol = clamp( strokeCol, 0, 1 );

    float finalHatchVal = strokeCol.r * 1.5;
    if( strokeCol.r > 0.5 )
        finalHatchVal = 1;

    vec3 white = vec3( 1, 1, 1 );
    vec3 black = vec3( 0, 0, 0 );
    return mix( black, white, finalHatchVal );
}


vec3 Contours() // this name is the one which is detected by the SetupShaders() function in the main application, and the one used to swap subroutines
{
    vec3 color = vec3(1.0, 1.0, 1.0);
    // Contours and Suggestive Contours Calculation considering feature size of the mesh
	float curvatureFs = feature_size * abs( curvatureValue );
	float derivateFs = feature_size * feature_size * curvatureDerivate;
	float derivateFsNV = ( derivateFs - derivateFs *  dotNV * dotNV );
	float contourLimitAfterFs = contourLimit * ( ( dotNV * dotNV ) / curvatureFs );
    float suggestivecLimitAfterFs = suggestivecLimit * ( curvatureFs / derivateFsNV );
	// This fragment is a contour
	if( enableContours && contourLimitAfterFs < 1.0 )
        color = strokeColor;
	// This fragment is a suggestive contour
	else if( enableSuggestiveContours && ( suggestivecLimitAfterFs < 1.0 ) && derivateFsNV > directionalDerivativeLimit ) 
        color = strokeColor;
	return color;

    
}

vec3 applyGrassTexture(){
    vec2 grassCoord = 0.10f * fragPos.xz;
    //vec3 terrainColor = mix(warmColor, coldColor, 0.5);
    return texture2D( grassTexture, grassCoord ).rgb; //* terrainColor;
}
//////////////////////////////////////////


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

    if (enableHatching){
        color *= DynamicSolidHatch();
    }

    /// base color
	if  (enableGrassTexture){
        color = color * mix(warmColor, coldColor, 0.3) * applyGrassTexture();
    }

    if (enableContours || enableSuggestiveContours){
        vec3 potentialColor = Contours();
        if(potentialColor == strokeColor){
            color *= Contours();
        }
    }

    //Final Fragment Color
    out_Color = vec4(color, 1.0);
}
