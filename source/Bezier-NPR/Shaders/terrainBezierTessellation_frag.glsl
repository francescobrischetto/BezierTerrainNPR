#version 410 core
out vec4 FragColor;


in vec2 TexCoords;
in vec3 FragPos;

in float mean_curv;

in vec4 LFragPos;
in vec3 LNormal;
in mat3 TBN;



uniform vec3 viewPos;
uniform vec3 lightPos;


uniform sampler2D texture_normal1;





vec4 pickColor(float s, float h, vec2 tCoords);
vec3 pickNormalMap(float s, vec2 tCoords);


void main()
{

    vec2 texCoords = TexCoords;



    vec3 TangentLightPos = TBN * lightPos;
    vec3 TangentViewPos = TBN * viewPos;
    vec3 TangentFragPos = TBN * FragPos;


    float slope = 1.0f - LNormal.y;
    //float slope = 1.0f - TangentLightPos.y;
    float elevation = mix(0.0, 1.0, LFragPos.y);
    vec4 color = pickColor(slope, elevation, texCoords);
    vec3 normal = pickNormalMap(slope, texCoords);
    //normal = normalize(normal * 2.0 - 1.0);
    normal = normalize(normal);
    vec4 ambient = 0.4 * color;







    vec3 lightDir = normalize(TangentLightPos - TangentFragPos);
    vec3 viewDir = normalize(TangentViewPos - TangentFragPos);    
    float diff = max(dot(lightDir, normal), 0.0);
    vec4 diffuse = diff * color;

   // specular
    vec3 reflectDir = reflect(-lightDir, normal);  
    vec3 halfwayDir = normalize(lightDir + viewDir);  
    float spec = pow(max(dot(normal, halfwayDir), 0.0), 32.0);
    vec3 specular = vec3(0.05) * spec;
 


    color = (ambient + diffuse + vec4(specular, 1.0));
    FragColor = color;
    FragColor = vec4(mean_curv,mean_curv,mean_curv,1.0);
    if(mean_curv < -0.05){
      FragColor = vec4(0.0,0.0,1.0,1.0);
    }
    else if(mean_curv >= -0.05 && mean_curv < -0.03){
      FragColor = vec4(0.0,1.0,1.0,1.0);
    }
    else if(mean_curv >= -0.03 && mean_curv < -0.01){
      FragColor = vec4(0.0,1.0,0.0,1.0);
    }
    else if(mean_curv >= -0.01 && mean_curv < 0.01){
      FragColor = vec4(1.0,1.0,0.0,1.0);
    }
    else{
      FragColor = vec4(1.0,0.0,0.0,1.0);
    }

}

vec4 pickColor(float s, float h, vec2 tCoords)
{

    const vec4 grassColor = vec4(0.133, 0.545, 0.133, 1.0);
    const vec4 rockColor = vec4(0.55, 0.55, 0.55, 1.0);

    
  vec4 c;

    if (h < 0.60)
    {
      float b = h / 0.60;
      c = mix(grassColor, rockColor, b);
    }
   //if (s > 0.30)
     // c = rockColor;

    //if (h < 0.20)
     // c = grassColor;
     else
    c = rockColor;


  



  //c = rockColor;
  return c;
}

vec3 pickNormalMap(float s, vec2 tCoords)
{


    vec3 grassNormal = texture(texture_normal1, tCoords).rgb;
    //vec3 rockNormal = texture(texture_normal1, tCoords).rgb;

    vec3 n = vec3(grassNormal.r * 2.0 - 1.0,  grassNormal.g * 2.0 - 1.0, grassNormal.b);

    //if (s <= 0.2f)
      //  n = grassNormal;
    //else
      //  n = rockNormal;
    
    return n;


}

