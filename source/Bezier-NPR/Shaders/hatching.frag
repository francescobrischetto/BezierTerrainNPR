#version 410 core

uniform sampler2D hatch1;
uniform sampler2D hatch2;
uniform sampler2D hatch3;
uniform sampler2D hatch4;
uniform sampler2D hatch5;
uniform sampler2D hatch6;
uniform mat4 viewMatrix;

uniform vec3 pointLightPosition;
uniform vec2 resolution;

in float nDotVP;
in vec3 vNormal;
in vec3 vPosition;

out vec4 colorFrag;

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
//////////////////////////////////////////

vec4 shade(vec2 vUv) {

    float diffuse = nDotVP;
    float specular = 0.;
    float ambient = 1.;
    int shininess = 49;
    float ambientWeight = 0.08;
    float diffuseWeight = 1.0;
    float specularWeight = 1.0;
    float rimWeight = 0.46;
    vec4 inkColor = vec4(0.0,0.0,0.0,1.0);

    vec3 n = normalize( vNormal );

    vec3 r = -reflect(( viewMatrix * vec4(pointLightPosition, 1.0) ).xyz, n);
    r = normalize(r);
    vec3 v = -vPosition.xyz;
    v = normalize(v);
    float nDotHV = max( 0., dot( r, v ) );

    if( nDotVP != 0. ) specular = pow ( nDotHV, shininess );
    float rim = max( 0., abs( dot( n, normalize( -vPosition.xyz ) ) ) );

    float shading = ambientWeight * ambient + diffuseWeight * diffuse + rimWeight * rim + specularWeight * specular;    
    vec4 c = vec4( 1. ,1., 1., 1. );
    float step = 1. / 6.;
    if( shading <= step ){
        c = mix( texture2D( hatch6, vUv ), texture2D( hatch5, vUv ), 6. * shading );
        //c = vec4( step );
    }
    if( shading > step && shading <= 2. * step ){
        c = mix( texture2D( hatch5, vUv ), texture2D( hatch4, vUv) , 6. * ( shading - step ) );
        //c = vec4( 2. * step );
    }
    if( shading > 2. * step && shading <= 3. * step ){
        c = mix( texture2D( hatch4, vUv ), texture2D( hatch3, vUv ), 6. * ( shading - 2. * step ) );
        //c = vec4( 3. * step );
    }
    if( shading > 3. * step && shading <= 4. * step ){
        c = mix( texture2D( hatch3, vUv ), texture2D( hatch2, vUv ), 6. * ( shading - 3. * step ) );
        //c = vec4( 4. * step );
    }
    if( shading > 4. * step && shading <= 5. * step ){
        c = mix( texture2D( hatch2, vUv ), texture2D( hatch1, vUv ), 6. * ( shading - 4. * step ) );
        //c = vec4( 5. * step );
    }
    if( shading > 5. * step ){
        c = mix( texture2D( hatch1, vUv ), vec4( 1. ), 6. * ( shading - 5. * step ) );
        //c = vec4( 6. * step );
    }

    vec4 src = mix( mix( inkColor, vec4( 1. ), c.r ), c, .5 );
    //c = 1. - ( 1. - src ) * ( 1. - dst );
    //c = vec4( min( src.r, dst.r ), min( src.g, dst.g ), min( src.b, dst.b ), 1. );

    //c = vec4( gl_FragCoord.x / resolution.x, gl_FragCoord.y / resolution.y, 0., 1. );

    return src;
}




// main (entry-point)
void main()
{
    vec2 uv = gl_FragCoord.xy / resolution;
    vec3 color = Illumination_Model();
    vec4 inkColor = vec4(0.0,0.0,0.0,1.0);
    vec4 src = ( .5 * inkColor ) * vec4( 1.0 ) + vec4( 1.0 ) * shade(uv);
    colorFrag = vec4( src.rgb, 1. );
}

