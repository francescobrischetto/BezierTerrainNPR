#version 410 core

// Define the type of input patch, a grid of 16 control points
layout(quads, equal_spacing, ccw) in;

out float normalDotViewValue;
// Normal in view coordinates
out vec3 viewNormal;
// Light direction in view coordinates
out vec3 viewLightDirection;
// Vector to Camera in view coordinate
out vec3 vectorToCamera;

// Structure to pass data to Tessellation Evaluation Shader
out CURVATURE_INFO{
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



uniform mat3 normalMatrix;
uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;
// Point Light Position in world Space
uniform vec3 pointLightWorldPosition;


vec2 calculateCurvaturePairFromFirstSecondFormMatrix(mat2 firstFundamentalFormMatrix, mat2 secondFundamentalFormMatrix){
    // Inverse of the first Fundamental Form Matrix
    mat2 firstTraspose = inverse(firstFundamentalFormMatrix); 
    // 2x2 multiplication matrix between second form and inverse of first form
    mat2 resultingMultiplicationMatrix = secondFundamentalFormMatrix * firstTraspose;
    // Curvature Values
    float k1 = 0;       float k2 = 0;
    // EigenValues Calculation of 2x2 result matrix (application of simple matrix algebra to find eigenvalues)
    if (resultingMultiplicationMatrix[1][0] == 0 ){
        k1 = resultingMultiplicationMatrix[0][0];
        k2 = resultingMultiplicationMatrix[1][1];
    }
    else if (resultingMultiplicationMatrix[0][0] == 0 ){
        k1 = resultingMultiplicationMatrix[0][1];
        k2 = resultingMultiplicationMatrix[1][0];
    }
    else{
        k1 = resultingMultiplicationMatrix[0][0];
        k2 = resultingMultiplicationMatrix[1][1] 
           - resultingMultiplicationMatrix[1][0]/resultingMultiplicationMatrix[0][0] 
           * resultingMultiplicationMatrix[0][1];
    }

    vec2 curvatureValues = vec2(k1,k2);
    return curvatureValues;
}

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

vec3 calculateCurvaturePairFromFirstSecondFormMatrix(mat2 firstFundamentalFormMatrix, mat2 secondFundamentalFormMatrix, float eigenValue, vec3 tangentVector, vec3 bitangentVector){
    // Inverse of the first Fundamental Form Matrix
    mat2 firstTraspose = inverse(firstFundamentalFormMatrix); 
    // 2x2 multiplication matrix between second form and inverse of first form
    mat2 resultingMultiplicationMatrix = secondFundamentalFormMatrix * firstTraspose;
    // EigenVector (principal curvature direction) in tangent Space
    vec2 eigenVector = estimateEigenVector(resultingMultiplicationMatrix, eigenValue);
    // EigenVector (principal curvature direction) in 3D Space
    vec3 resultingEigenVector = eigenVector.x * normalize(tangentVector) + eigenVector.y * normalize(bitangentVector);
    return resultingEigenVector;
}


mat3 ComputeTangentBitangentNormalMatrix(vec3 tangentVector, vec3 bitangentVector, vec3 normalVector){
    //mat3 normalMatrix = transpose(inverse(mat3(modelMatrix)));
    vec3 T = normalize(normalMatrix * tangentVector);
    vec3 N = normalize(normalMatrix * normalVector);
    vec3 B = normalize(normalMatrix * bitangentVector);
    T = normalize(T - dot(T, N) * B);
    B = cross(N, T);
    //  TBN Matrix
    mat3 TBN = inverse(mat3(T, B, N));
    return TBN;
}

void main()
{  
    // We get all the 16 Control Points
    vec4 p00 = gl_in[0].gl_Position;    vec4 p01 = gl_in[4].gl_Position;
    vec4 p10 = gl_in[1].gl_Position;    vec4 p11 = gl_in[5].gl_Position;
    vec4 p20 = gl_in[2].gl_Position;    vec4 p21 = gl_in[6].gl_Position;
    vec4 p30 = gl_in[3].gl_Position;    vec4 p31 = gl_in[7].gl_Position;
    
    vec4 p02 = gl_in[8].gl_Position;    vec4 p03 = gl_in[12].gl_Position;
    vec4 p12 = gl_in[9].gl_Position;    vec4 p13 = gl_in[13].gl_Position;
    vec4 p22 = gl_in[10].gl_Position;   vec4 p23 = gl_in[14].gl_Position;
    vec4 p32 = gl_in[11].gl_Position;   vec4 p33 = gl_in[15].gl_Position;
    
    // We get the U,V coords
    float u = gl_TessCoord.x;
    float v = gl_TessCoord.y;
    curvature_informations.uvCoordinatesInBezierPatch = vec2(u, v);

    // U - weights for bezier surface                           // V - weights for bezier surface
    float bu0 = (1-u) * (1-u) * (1-u);                          float bv0 = (1-v) * (1-v) * (1-v);
    float bu1 = 3 * u * (1-u) * (1-u);                          float bv1 = 3 * v * (1-v) * (1-v);
    float bu2 = 3 * u * u * (1-u);                              float bv2 = 3 * v * v * (1-v);
    float bu3 = u * u * u;                                      float bv3 = v * v * v;
    
    // U - derivate weights for bezier surface                  // V - derivate weights for bezier surface 
    float dbu0 = -3 * (1-u) * (1-u);                            float dbv0 = -3 * (1-v) * (1-v);
    float dbu1 = (3 * (1-u) * (1-u)) - ((6*u) * (1-u));         float dbv1 = (3 * (1-v) * (1-v)) - ((6*v) * (1-v));
    float dbu2 = ((6*u) * (1 - u)) - (3 * u * u);               float dbv2 = ((6*v) * (1 - v)) - (3 * v * v); 
    float dbu3 = 3 * u * u;                                     float dbv3 = 3 * v * v;

    // U - double derivate weights for bezier surface           // V - double derivate weights for bezier surface 
    float ddbu0 = 6 * (1-u) ;                                   float ddbv0 = 6 * (1-v) ;
    float ddbu1 = (-6 * (1-u)) - (6 * (1-u) - 6 * u);           float ddbv1 = (-6 * (1-v)) - (6 * (1-v) - 6 * v);
    float ddbu2 = (6 * (1 - u)) - (6 * u) - (6 * u);            float ddbv2 = (6 * (1 - v)) - (6 * v) - (6 * v); 
    float ddbu3 = 6 * u;                                        float ddbv3 = 6 * v;

    // Calculation of the position in the bezier patch using weights and control points
    vec4 vertexPosition = bu0 * ( bv0*p00 + bv1*p01 + bv2*p02 + bv3*p03 )
                        + bu1 * ( bv0*p10 + bv1*p11 + bv2*p12 + bv3*p13 )
                        + bu2 * ( bv0*p20 + bv1*p21 + bv2*p22 + bv3*p23 )
                        + bu3 * ( bv0*p30 + bv1*p31 + bv2*p32 + bv3*p33 );

    // Calculation of tangent/bitangent vectors in the bezier patch using derivate weights and control points
    vec3 tangentVector = (dbu0 * ( bv0*p00 + bv1*p01 + bv2*p02 + bv3*p03 )
                       + dbu1 * ( bv0*p10 + bv1*p11 + bv2*p12 + bv3*p13 )
                       + dbu2 * ( bv0*p20 + bv1*p21 + bv2*p22 + bv3*p23 )
                       + dbu3 * ( bv0*p30 + bv1*p31 + bv2*p32 + bv3*p33 )).xyz;

    vec3 bitangentVector = (bu0 * ( dbv0*p00 + dbv1*p01 + dbv2*p02 + dbv3*p03 )
                         + bu1 * ( dbv0*p10 + dbv1*p11 + dbv2*p12 + dbv3*p13 )
                         + bu2 * ( dbv0*p20 + dbv1*p21 + dbv2*p22 + dbv3*p23 )
                         + bu3 * ( dbv0*p30 + dbv1*p31 + dbv2*p32 + dbv3*p33 )).xyz;

    // Computation of Normal vector (cross product + normalization of tangent vectors)
    vec3 normalVector = normalize(cross( tangentVector.xyz, bitangentVector.xyz ));

    // Computation of partial derivatives for second fundamental form Matrix
    vec3 secondPartialDerivativeUU = (ddbu0 * ( bv0*p00 + bv1*p01 + bv2*p02 + bv3*p03 )
                                   + ddbu1 * ( bv0*p10 + bv1*p11 + bv2*p12 + bv3*p13 )
                                   + ddbu2 * ( bv0*p20 + bv1*p21 + bv2*p22 + bv3*p23 )
                                   + ddbu3 * ( bv0*p30 + bv1*p31 + bv2*p32 + bv3*p33 )).xyz;

    vec3 secondPartialDerivativeUV = (dbu0 * ( dbv0*p00 + dbv1*p01 + dbv2*p02 + dbv3*p03 )
                                   + dbu1 * ( dbv0*p10 + dbv1*p11 + dbv2*p12 + dbv3*p13 )
                                   + dbu2 * ( dbv0*p20 + dbv1*p21 + dbv2*p22 + dbv3*p23 )
                                   + dbu3 * ( dbv0*p30 + dbv1*p31 + dbv2*p32 + dbv3*p33 )).xyz;

    vec3 secondPartialDerivativeVV = (bu0 * ( ddbv0*p00 + ddbv1*p01 + ddbv2*p02 + ddbv3*p03 )
                                   + bu1 * ( ddbv0*p10 + ddbv1*p11 + ddbv2*p12 + ddbv3*p13 )
                                   + bu2 * ( ddbv0*p20 + ddbv1*p21 + ddbv2*p22 + ddbv3*p23 )
                                   + bu3 * ( ddbv0*p30 + ddbv1*p31 + ddbv2*p32 + ddbv3*p33 )).xyz;
    
    // First Fundamental Form Matrix
    float E = dot( tangentVector,tangentVector );
    float F = dot( tangentVector,bitangentVector );
    float G = dot( bitangentVector,bitangentVector );
    curvature_informations.firstFundamentalFormMatrix = mat2(E, F, F, G);

    // Second Fundamental Form Matrix
    float L = dot(normalVector,secondPartialDerivativeUU);
    float M = dot(normalVector,secondPartialDerivativeUV);
    float N = dot(normalVector,secondPartialDerivativeVV);
    curvature_informations.secondFundamentalFormMatrix = mat2(L, M, M, N);

    // Calculation of principal curvatures, k1 and k2, values given First and Second Fundamental Form Matrices
    vec2 curvatureValues = calculateCurvaturePairFromFirstSecondFormMatrix(curvature_informations.firstFundamentalFormMatrix, curvature_informations.secondFundamentalFormMatrix);
    curvature_informations.k1 = curvatureValues.x;       curvature_informations.k2 = curvatureValues.y;
    // Calculation of Mean and Gaussian Curvature based on principal curvatures, k1 and k2, previously calculated

    // Mean Curvature Computation        { Can be also computed directly as: ( E*N - 2*F*M + G*L ) / ( 2*( E*G - F*F ) ) }
    curvature_informations.meanCurvature = curvature_informations.k1 + curvature_informations.k2 / 2;

    // Gaussian Curvature Computation    { Can be also computed directly as: ( L*N - M*M ) / ( E*G - F*F ) }
    curvature_informations.gaussianCurvature = curvature_informations.k1 * curvature_informations.k2;

    // Calculation of principal curvatures, pdir1 and pdir2, directions in 3D space, given EigenValue, Tangent vector, Bitangent vector, First and Second Fundamental Form Matrices
    curvature_informations.principalDirection1 = calculateCurvaturePairFromFirstSecondFormMatrix(curvature_informations.firstFundamentalFormMatrix, curvature_informations.secondFundamentalFormMatrix, curvature_informations.k1, tangentVector, bitangentVector);
    curvature_informations.principalDirection2 = calculateCurvaturePairFromFirstSecondFormMatrix(curvature_informations.firstFundamentalFormMatrix, curvature_informations.secondFundamentalFormMatrix, curvature_informations.k2, tangentVector, bitangentVector);

    vec4 mvPosition = viewMatrix * modelMatrix * vertexPosition;
    // Calculation of vector to camera
	vectorToCamera = normalize(-mvPosition.xyz);

    //normalVector è il versore normale del piano tangente
    //So if you have a vector A and a plane with normal N, the vector that is resulted by projecting A on the plane will be B = A - (A.dot.N)N
    curvature_informations.viewVectorProjectedInTangentPlane = vectorToCamera - normalVector * dot(vectorToCamera, normalVector);
    //Now I need w to be expressed in tangent coordinate system
    curvature_informations.TBN = ComputeTangentBitangentNormalMatrix(tangentVector, bitangentVector, normalVector);
    //  view Vector Projected in Tangent Plane expressed in Tangent Coordinate System
    curvature_informations.w = (curvature_informations.TBN * curvature_informations.viewVectorProjectedInTangentPlane).xy;

    // Inverse of the first Fundamental Form Matrix
    mat2 firstTraspose = inverse(curvature_informations.firstFundamentalFormMatrix); 
    // 2x2 multiplication matrix between second form and inverse of first form
    mat2 weingartenMatrix = curvature_informations.secondFundamentalFormMatrix * firstTraspose;

    //The normal curvature of a surface S at a point p measures its curvature in a specific direction x in the tangent plane
    curvature_informations.normalCurvatureInDirectionW = ( dot(( curvature_informations.secondFundamentalFormMatrix * curvature_informations.w ), curvature_informations.w)/dot(curvature_informations.w,curvature_informations.w) );

	// compute ndotv
	normalDotViewValue = max(dot(normalVector,vectorToCamera), 0.0);

    // Light position in view coordinates
    vec4 lightPos = viewMatrix  * vec4(pointLightWorldPosition, 1.0);
    // Light vector in view coordinates
    viewLightDirection = lightPos.xyz - mvPosition.xyz;

    viewNormal = normalize(normalMatrix * normalVector);

    //passing position to fragment Shader
    gl_Position = projectionMatrix * viewMatrix * modelMatrix * vertexPosition;
}

