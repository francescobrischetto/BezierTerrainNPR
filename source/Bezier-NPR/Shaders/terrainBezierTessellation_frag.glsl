#version 410 core

out vec4 FragColor;

in vec3 viewVectorProjectedInTangentPlane;
in float normalCurvatureInDirectionW;
in float normalDotViewValue;


float c_limit = 0.1;


void main()
{

    vec4 color = vec4(0.6,0.6,0.6, 1.0);
    float contour_limit = (pow(normalDotViewValue, 2.0));
    // Derivate of normal Curvature in direction W
    float derivateNormalCurvatureInDirectionW = viewVectorProjectedInTangentPlane.x * dFdx(normalCurvatureInDirectionW) + viewVectorProjectedInTangentPlane.y * dFdy(normalCurvatureInDirectionW);
    if(contour_limit<c_limit)
      {FragColor = vec4(min(color.xyz, vec3(contour_limit, contour_limit, contour_limit)),1.0);}
    else{
      if(normalCurvatureInDirectionW >= -0.001 && normalCurvatureInDirectionW < 0.001 && derivateNormalCurvatureInDirectionW>0){
      //if(normalCurvatureInDirectionW == 0 && derivateNormalCurvatureInDirectionW>0){
        FragColor = mix(vec4(1.0), vec4(0.0),0.5);
        //FragColor = vec4(0.0,0.0,0.0,1.0);
      }else{
        //FragColor = mix(vec4(0.15,0.15,0.15,1.0),color,clamp((abs(normalCurvatureInDirectionW))*500,0,1));
        FragColor = vec4(1.0,1.0,1.0,1.0);
      }
    }

}
