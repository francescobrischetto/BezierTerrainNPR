#version 410 core

// Tessellation is working with triangles
layout(vertices=3) out;

//Uniforms
// Camera in world space coordinate
uniform vec3 cameraWorldPosition;

// Structure to take data from Vertex Shader
in VS_OUT{
    vec3 position;
    vec3 normal;
    float curv1;
    float curv2;
    float feature_size;
    vec3 pdir1;
    vec3 pdir2;
    vec4 dcurv;
} tcs_in [];

// Structure to pass data to Tessellation Evaluation Shader
out TCS_OUT{
    vec3 position;
    vec3 normal;
    float curv1;
    float curv2;
    float feature_size;
    vec3 pdir1;
    vec3 pdir2;
    vec4 dcurv;
} tcs_out [];


// Function that adjust tessellation level based on distance from camera
float GetTessLevel(float d0, float d1)                                            
{                                                                                               
    float AvgDistance = (d0 + d1) / 2.0;
    // Tessellation level decrease with distance                                                                                                                                    
    if      ( AvgDistance <= 3.0  )    return 15.0;                                                                                                                                                                      
    else if ( AvgDistance <= 5.0  )    return 12.0;                                                              
    else if ( AvgDistance <= 10.0 )    return 7.0;                                                                                                                                              
    else if ( AvgDistance <= 25.0 )    return 4.0;                                                                                                                                                     
    else                               return 1.0;                                                                                                                                                                       
}

void main(){

    // We pass all the data to the tes
    tcs_out[gl_InvocationID].position       = tcs_in[gl_InvocationID].position;
    tcs_out[gl_InvocationID].normal         = tcs_in[gl_InvocationID].normal;
    tcs_out[gl_InvocationID].curv1          = tcs_in[gl_InvocationID].curv1;
    tcs_out[gl_InvocationID].curv2          = tcs_in[gl_InvocationID].curv2;
    tcs_out[gl_InvocationID].feature_size   = tcs_in[gl_InvocationID].feature_size;
    tcs_out[gl_InvocationID].pdir1          = tcs_in[gl_InvocationID].pdir1;
    tcs_out[gl_InvocationID].pdir2          = tcs_in[gl_InvocationID].pdir2;
    tcs_out[gl_InvocationID].dcurv          = tcs_in[gl_InvocationID].dcurv;

    // Calculate the distance from the camera (worldSpace) to the three control points (worldSpace, previously transformed)                      
    float cameraToVertexDistance0 = distance(cameraWorldPosition, tcs_out[0].position);                     
    float cameraToVertexDistance1 = distance(cameraWorldPosition, tcs_out[1].position);                     
    float cameraToVertexDistance2 = distance(cameraWorldPosition, tcs_out[2].position);
                                                                                        
    // Assign the correct tessellation level to inner and outer triangles                                                    
    gl_TessLevelOuter[0] = GetTessLevel(cameraToVertexDistance1, cameraToVertexDistance2);            
    gl_TessLevelOuter[1] = GetTessLevel(cameraToVertexDistance2, cameraToVertexDistance0);            
    gl_TessLevelOuter[2] = GetTessLevel(cameraToVertexDistance0, cameraToVertexDistance1);            
    gl_TessLevelInner[0] = gl_TessLevelOuter[2];   

}
                                                                                         