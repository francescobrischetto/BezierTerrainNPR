#version 410 core

uniform sampler2D hatch1;
uniform sampler2D hatch2;
uniform sampler2D hatch3;
uniform sampler2D hatch4;
uniform vec3 gEyeWorldPos;


in vec4 fragPos;
in vec3 fragNor;
in vec3 lightDir;
in vec3 vViewPosition;

out vec4 color;

// the "type" of the Subroutine
subroutine vec3 ill_model();
// Subroutine Uniform (it is conceptually similar to a C pointer function)
subroutine uniform ill_model Illumination_Model;

///////////////////ILLUMINATION MODELS///////////////////////
// a subroutine for the Lambert model
subroutine(ill_model)
vec3 Lambert() // this name is the one which is detected by the SetupShaders() function in the main application, and the one used to swap subroutines
{  
    return vec3(1.0,0.0,0.0);
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


// calculate light value at the current fragment
float GetLightValue() 
{
    vec3 N = normalize(fragNor.xyz);
    vec3 L = normalize(lightDir.xyz);
    float lambertian = max(dot(L,N), 0.0);
    vec3 V = normalize( vViewPosition );
	vec3 H = normalize(L + V);
    float specAngle = max(dot(H, N), 0.0);
    float specular = pow(specAngle, 20);
	float ambColor  = 0.2f;
    float diffColor = 0.9f * lambertian;
    float specColor = 0.1f * specular;
    return clamp(  diffColor + specColor, ambColor, 1 );
}


// Dynamic solid texture implementation for blending hatch/stroke 
// textures at various distances
vec4 GetDynamicSolidColor( vec3 pos ) 
{
    float zcam = distance( fragPos.xyz, gEyeWorldPos.xyz );
    float s = log2( zcam ) - floor( log2( zcam ) );
    vec3 uv = ( 1.5 * pos / pow( 2.0, floor( log2( zcam ) ) ) );
    
    vec3 normal = normalize( fragNor.xyz );
    float tone = max( 0, 1 - GetLightValue() );

    //float uvNoiseY = texture2D( hatchNoiseTex, uv1.xy / 10.f ).r * 0.2;
    //uv1 += vec3( 0, uvNoiseY, 0 );

    vec2 uv1 = 2.0f * uv.xy;
    vec2 uv2 = 2 * uv1;
    vec2 uv3 = 4 * uv1;
    vec2 uv4 = 8 * uv1;
    
    float a1 = s / 2.0;
    float a2 = 0.5 - (s / 6.0);
    float a3 = (1.0 / 3.0) - (s / 6.0);
    float a4 = (1.0 / 6.0) - (s / 6.0);

    vec4 tex0 = texture( hatch1, uv1 ) * a1 + texture( hatch1, uv2 ) * a2 +
                texture( hatch1, uv3 ) * a3 + texture( hatch1, uv4 ) * a4;
    //vec4 tex1 = texture( hatch2, uv1 ) * a1 + texture( hatch2, uv2 ) * a2 +
    //            texture( hatch2, uv3 ) * a3 + texture( hatch2, uv4 ) * a4;
    //vec4 tex2 = texture( hatch3, uv1 ) * a1 + texture( hatch3, uv2 ) * a2 +
    //            texture( hatch3, uv3 ) * a3 + texture( hatch3, uv4 ) * a4;
    //vec4 tex3 = texture( hatch4, uv1 ) * a1 + texture( hatch4, uv2 ) * a2 +
    //            texture( hatch4, uv3 ) * a3 + texture( hatch4, uv4 ) * a4;
    vec4 tex1 = GetBlendedHatch( hatch1, 2, uv1, false ) * a1 + GetBlendedHatch( hatch1, 2, uv2, false ) * a2 +
                GetBlendedHatch( hatch1, 2, uv3, false ) * a3 + GetBlendedHatch( hatch1, 2, uv4, false ) * a4;
    vec4 tex2 = GetBlendedHatch( hatch1, 3, uv1, false ) * a1 + GetBlendedHatch( hatch1, 3, uv2, false ) * a2 +
            GetBlendedHatch( hatch1, 3, uv3, false ) * a3 + GetBlendedHatch( hatch1, 3, uv4, false ) * a4;
    vec4 tex3 = GetBlendedHatch( hatch1, 4, uv1, true ) * a1 + GetBlendedHatch( hatch1, 4, uv2, true ) * a2 +
            GetBlendedHatch( hatch1, 4, uv3, true ) * a3 + GetBlendedHatch( hatch1, 4, uv4, true ) * a4;

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

vec4 ColorDynamicSolidHatch()
{
    vec3 normal = normalize( fragNor );
    vec3 strokeCol = GetDynamicSolidColor( vec3( fragPos.xz, fragPos.y ) ).rgb;
    strokeCol = clamp( strokeCol, 0, 1 );

    float finalHatchVal = strokeCol.r * 1.5;
    if( strokeCol.r > 0.5 )
        finalHatchVal = 1;

    vec4 white = vec4( 1, 1, 1, 1 );
    vec4 black = vec4( 0, 0, 0, 1 );
    return mix( black, white, finalHatchVal );
}

// main (entry-point)
void main()
{
    color = vec4( 1 );
    vec3 color2 = Illumination_Model();
    color *= ColorDynamicSolidHatch();
}