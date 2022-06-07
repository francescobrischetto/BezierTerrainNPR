#version 410 core

layout(quads, equal_spacing, ccw) in;

out vec4 LFragPos;
out vec3 LNormal;
out vec3 FragPos;
out vec2 TexCoords;


out mat3 TBN;
out float mean_curv;

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;






void main()
{
    vec4 p00 = gl_in[0].gl_Position;
    vec4 p10 = gl_in[1].gl_Position;
    vec4 p20 = gl_in[2].gl_Position;
    vec4 p30 = gl_in[3].gl_Position;
    vec4 p01 = gl_in[4].gl_Position;
    vec4 p11 = gl_in[5].gl_Position;
    vec4 p21 = gl_in[6].gl_Position;
    vec4 p31 = gl_in[7].gl_Position;
    vec4 p02 = gl_in[8].gl_Position;
    vec4 p12 = gl_in[9].gl_Position;
    vec4 p22 = gl_in[10].gl_Position;
    vec4 p32 = gl_in[11].gl_Position;
    vec4 p03 = gl_in[12].gl_Position;
    vec4 p13 = gl_in[13].gl_Position;
    vec4 p23 = gl_in[14].gl_Position;
    vec4 p33 = gl_in[15].gl_Position;

    float u = gl_TessCoord.x;
    float v = gl_TessCoord.y;
    TexCoords = vec2(u, v);


    float bu0 = (1-u) * (1-u) * (1-u);
    float bu1 = 3 * u * (1-u) * (1-u);
    float bu2 = 3 * u * u * (1-u);
    float bu3 = u * u * u;
    

    float bv0 = (1-v) * (1-v) * (1-v);
    float bv1 = 3 * v * (1-v) * (1-v);
    float bv2 = 3 * v * v * (1-v);
    float bv3 = v * v * v;
    
    float dbu0 = -3 * (1-u) * (1-u);
    float dbu1 = (3 * (1-u) * (1-u)) - ((6*u) * (1-u));
    float dbu2 = ((6*u) * (1 - u)) - (3 * u * u); 
    float dbu3 = 3 * u * u;

    float ddbu0 = 6 * (1-u) ;
    float ddbu1 = (-6 * (1-u)) - (6 * (1-u) - 6 * u);
    float ddbu2 = (6 * (1 - u)) - (6 * u) - (6 * u); 
    float ddbu3 = 6 * u;


    
    float dbv0 = -3 * (1-v) * (1-v);
    float dbv1 = (3 * (1-v) * (1-v)) - ((6*v) * (1-v));
    float dbv2 = ((6*v) * (1 - v)) - (3 * v * v); 
    float dbv3 = 3 * v * v;

    float ddbv0 = 6 * (1-v) ;
    float ddbv1 = (-6 * (1-v)) - (6 * (1-v) - 6 * v);
    float ddbv2 = (6 * (1 - v)) - (6 * v) - (6 * v); 
    float ddbv3 = 6 * v;

     vec4 position = bu0 * ( bv0*p00 + bv1*p01 + bv2*p02 + bv3*p03 )
    + bu1 * ( bv0*p10 + bv1*p11 + bv2*p12 + bv3*p13 )
    + bu2 * ( bv0*p20 + bv1*p21 + bv2*p22 + bv3*p23 )
    + bu3 * ( bv0*p30 + bv1*p31 + bv2*p32 + bv3*p33 );

  

    vec4 dpdu = dbu0 * ( bv0*p00 + bv1*p01 + bv2*p02 + bv3*p03 )
    + dbu1 * ( bv0*p10 + bv1*p11 + bv2*p12 + bv3*p13 )
    + dbu2 * ( bv0*p20 + bv1*p21 + bv2*p22 + bv3*p23 )
    + dbu3 * ( bv0*p30 + bv1*p31 + bv2*p32 + bv3*p33 );


    vec4 dpdv = bu0 * ( dbv0*p00 + dbv1*p01 + dbv2*p02 + dbv3*p03 )
    + bu1 * ( dbv0*p10 + dbv1*p11 + dbv2*p12 + dbv3*p13 )
    + bu2 * ( dbv0*p20 + dbv1*p21 + dbv2*p22 + dbv3*p23 )
    + bu3 * ( dbv0*p30 + dbv1*p31 + dbv2*p32 + dbv3*p33 );

    vec4 dpduu = ddbu0 * ( bv0*p00 + bv1*p01 + bv2*p02 + bv3*p03 )
    + ddbu1 * ( bv0*p10 + bv1*p11 + bv2*p12 + bv3*p13 )
    + ddbu2 * ( bv0*p20 + bv1*p21 + bv2*p22 + bv3*p23 )
    + ddbu3 * ( bv0*p30 + bv1*p31 + bv2*p32 + bv3*p33 );
    vec4 dpduv = dbu0 * ( dbv0*p00 + dbv1*p01 + dbv2*p02 + dbv3*p03 )
    + dbu1 * ( dbv0*p10 + dbv1*p11 + dbv2*p12 + dbv3*p13 )
    + dbu2 * ( dbv0*p20 + dbv1*p21 + dbv2*p22 + dbv3*p23 )
    + dbu3 * ( dbv0*p30 + dbv1*p31 + dbv2*p32 + dbv3*p33 );
    vec4 dpdvv = bu0 * ( ddbv0*p00 + ddbv1*p01 + ddbv2*p02 + ddbv3*p03 )
    + bu1 * ( ddbv0*p10 + ddbv1*p11 + ddbv2*p12 + ddbv3*p13 )
    + bu2 * ( ddbv0*p20 + ddbv1*p21 + ddbv2*p22 + ddbv3*p23 )
    + bu3 * ( ddbv0*p30 + ddbv1*p31 + ddbv2*p32 + ddbv3*p33 );

    
    vec3 aNormal = cross( dpdu.xyz, dpdv.xyz );
    LNormal = normalize(aNormal);

    float EE = dot(dpdu.xyz,dpdu.xyz);
    float FF = dot(dpdu.xyz,dpdv.xyz);
    float GG = dot(dpdv.xyz,dpdv.xyz);

    float LL = dot(dpduu.xyz,aNormal);
    float MM = dot(dpduv.xyz,aNormal);
    float NN = dot(dpdvv.xyz,aNormal);


    mat3 normalMatrix = transpose(inverse(mat3(modelMatrix)));
    vec3 T = normalize(normalMatrix * vec3(dpdu));
    vec3 N = normalize(normalMatrix * aNormal);
    //vec3 B = normalize(normalMatrix * vec3(dpdv));


    T = normalize(T - dot(T, N) * N);
    vec3 B = cross(N, T);
    TBN = transpose(mat3(T, B, N));


    //Mean Curvature Computation
    mean_curv = (EE*NN - 2*FF*MM + GG*LL) / (2*(EE*GG - FF*FF));
    //Gaussian Curvature Computation
    //mean_curv = (LL*NN - MM*MM) / (EE*GG - FF*FF);


    
    //Slope = 1.0f - N.y;
    LFragPos = position;
    
    
    FragPos = vec3(modelMatrix * position);
    //FragPos += N * displacement;
 
    gl_Position = projectionMatrix * viewMatrix * vec4(FragPos, 1.0);



}

