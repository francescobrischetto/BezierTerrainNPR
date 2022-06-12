#version 410 core

layout(quads, equal_spacing, ccw) in;

out vec3 LNormal;
out vec2 TexCoords;
out vec3 pdir1;
out vec3 pdir2;
out float mean_curv;

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;

// Inverse Power Iteration method to estimate eigenvector from eigenvalue
vec2 estimateEigenVector(mat2 matrix, float eigenValue){
    int count = 0;
    vec2 v = vec2(1.0, 0.0);
    vec2 prevVector = v;
    mat2 identityMatrix = mat2(1,0,0,1);

    while (count < 10)
	{
		// Store a copy of the current working vector to use for computing delta.
		prevVector = v;
		// Compute the next value of v.
		mat2 tempMatrix = matrix - (eigenValue * identityMatrix);
		tempMatrix = mat2(tempMatrix[1][1], -tempMatrix[0][1], -tempMatrix[1][0], tempMatrix[0][0] );
		v = tempMatrix * v;
		v = normalize(v);
		
		// Increment iteration count.
		count++;

	}

    return v;
}

void main()
{  
    // We get all the 16 Control Points
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

    // We get the U,V coords
    float u = gl_TessCoord.x;
    float v = gl_TessCoord.y;
    TexCoords = vec2(u, v);

    // U - weights for bezier surface 
    float bu0 = (1-u) * (1-u) * (1-u);
    float bu1 = 3 * u * (1-u) * (1-u);
    float bu2 = 3 * u * u * (1-u);
    float bu3 = u * u * u;
    
    // U - derivate weights for bezier surface 
    float dbu0 = -3 * (1-u) * (1-u);
    float dbu1 = (3 * (1-u) * (1-u)) - ((6*u) * (1-u));
    float dbu2 = ((6*u) * (1 - u)) - (3 * u * u); 
    float dbu3 = 3 * u * u;

    // U - double derivate weights for bezier surface 
    float ddbu0 = 6 * (1-u) ;
    float ddbu1 = (-6 * (1-u)) - (6 * (1-u) - 6 * u);
    float ddbu2 = (6 * (1 - u)) - (6 * u) - (6 * u); 
    float ddbu3 = 6 * u;

    // V - weights for bezier surface
    float bv0 = (1-v) * (1-v) * (1-v);
    float bv1 = 3 * v * (1-v) * (1-v);
    float bv2 = 3 * v * v * (1-v);
    float bv3 = v * v * v;

    // V - derivate weights for bezier surface 
    float dbv0 = -3 * (1-v) * (1-v);
    float dbv1 = (3 * (1-v) * (1-v)) - ((6*v) * (1-v));
    float dbv2 = ((6*v) * (1 - v)) - (3 * v * v); 
    float dbv3 = 3 * v * v;
    
    // V - double derivate weights for bezier surface 
    float ddbv0 = 6 * (1-v) ;
    float ddbv1 = (-6 * (1-v)) - (6 * (1-v) - 6 * v);
    float ddbv2 = (6 * (1 - v)) - (6 * v) - (6 * v); 
    float ddbv3 = 6 * v;

    // Calculation of the position in the bezier patch using weights and control points
     vec4 position = bu0 * ( bv0*p00 + bv1*p01 + bv2*p02 + bv3*p03 )
    + bu1 * ( bv0*p10 + bv1*p11 + bv2*p12 + bv3*p13 )
    + bu2 * ( bv0*p20 + bv1*p21 + bv2*p22 + bv3*p23 )
    + bu3 * ( bv0*p30 + bv1*p31 + bv2*p32 + bv3*p33 );

    // Calculation of tangent vectors in the bezier patch using derivate weights and control points
    vec4 dpdu = dbu0 * ( bv0*p00 + bv1*p01 + bv2*p02 + bv3*p03 )
    + dbu1 * ( bv0*p10 + bv1*p11 + bv2*p12 + bv3*p13 )
    + dbu2 * ( bv0*p20 + bv1*p21 + bv2*p22 + bv3*p23 )
    + dbu3 * ( bv0*p30 + bv1*p31 + bv2*p32 + bv3*p33 );

    vec4 dpdv = bu0 * ( dbv0*p00 + dbv1*p01 + dbv2*p02 + dbv3*p03 )
    + bu1 * ( dbv0*p10 + dbv1*p11 + dbv2*p12 + dbv3*p13 )
    + bu2 * ( dbv0*p20 + dbv1*p21 + dbv2*p22 + dbv3*p23 )
    + bu3 * ( dbv0*p30 + dbv1*p31 + dbv2*p32 + dbv3*p33 );

    // Computation of Normal vector (cross product + normalization of tangent vectors)
    vec3 aNormal = cross( dpdu.xyz, dpdv.xyz );
    LNormal = normalize(aNormal);

    // Computation of partial derivatives for second fundamental form Matrix
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

    
    // First Fundamental Form Matrix
    float EE = dot(dpdu.xyz,dpdu.xyz);
    float FF = dot(dpdu.xyz,dpdv.xyz);
    float GG = dot(dpdv.xyz,dpdv.xyz);
    mat2 firstForm = mat2(EE, FF, FF, GG);

    // Second Fundamental Form Matrix
    float LL = dot(LNormal,dpduu.xyz);
    float MM = dot(LNormal,dpduv.xyz);
    float NN = dot(LNormal,dpdvv.xyz);
    mat2 secondForm = mat2(LL, MM, MM, NN);
    mat2 firstTraspose = mat2(GG, -FF, -FF, EE);
    

    //Calcolo degli autovalori
    float detFirstTraspose = 1.0/(EE*GG - FF*FF);
    mat2 mulMatrix = detFirstTraspose * secondForm * firstTraspose;
    float k1 = 0;
    float k2 = 0;
    if (mulMatrix[1][0] == 0 ){
        k1 = mulMatrix[0][0];
        k2 = mulMatrix[1][1];
    }
    else if (mulMatrix[0][0] == 0 ){
        k1 = mulMatrix[0][1];
        k2 = mulMatrix[1][0];
    }
    else{
        k1 = mulMatrix[0][0];
        k2 = mulMatrix[1][1] - mulMatrix[1][0]/mulMatrix[0][0] * mulMatrix[0][1];
    }
    //Mean Curvature Computation
    //float mean_curv1 = (EE*NN - 2*FF*MM + GG*LL) / (2*(EE*GG - FF*FF));
    mean_curv = k1 * k2 / 2;
    //Gaussian Curvature Computation
    //mean_curv = (LL*NN - MM*MM) / (EE*GG - FF*FF);
    mean_curv = k1*k2;

    //Stima degli autovettori 
    vec2 v1 = estimateEigenVector(mulMatrix, k1);
    vec2 v2 = estimateEigenVector(mulMatrix, k2);

    pdir1 = v1.x * normalize(dpdu.xyz) + v1.y * normalize(dpdv.xyz);
    pdir2 = v2.x * normalize(dpdu.xyz) + v2.y * normalize(dpdv.xyz);

    

    gl_Position = projectionMatrix * viewMatrix * modelMatrix * position;



}

