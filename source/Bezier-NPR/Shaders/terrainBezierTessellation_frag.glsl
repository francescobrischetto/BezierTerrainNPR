#version 410 core

out vec4 FragColor;

in vec2 TexCoords;
in float mean_curv;
in vec3 LNormal;
in vec3 pdir1;
in vec3 pdir2;
in vec3 w;
in float kwn;

in float ndotv;
in float t_kr;
in float t_dwkr;

float c_limit = 0.1;
float sc_limit = 3.1;
float dwkr_limit = 0.05;


void main()
{
    /*
    vec2 texCoords = TexCoords;
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
    */
    vec4 color = vec4(0.6,0.6,0.6, 1.0);
    float contour_limit = (pow(ndotv, 2.0));
    float dFKwn = w.x * dFdx(kwn) + w.y * dFdy(kwn);
    if(contour_limit<c_limit)
      {FragColor = vec4(min(color.xyz, vec3(contour_limit, contour_limit, contour_limit)),1.0);}
    else{
      if(kwn >= -0.002 && kwn < 0.002 && dFKwn>0){
        FragColor = vec4(0.0,0.0,0.0,1.0);
      }else{
        FragColor = mix(vec4(0.15,0.15,0.15,1.0),color,clamp((abs(kwn))*500,0,1));
        //FragColor = vec4(1.0,1.0,1.0,1.0);
      }
    }
    //vec4 color = vec4(1.0f,1.0f,1.0f, 1.0f);
  

    /*
    // use feature size
    float kr = abs(t_kr); // absolute value to use it in limits
    float dwkr = t_dwkr; // two times fz because derivative
    float dwkr2 = (dwkr-dwkr*pow(ndotv, 2.0));

    // compute limits
    float contour_limit = c_limit*(pow(ndotv, 2.0)/kr);
    float sc_limit = sc_limit*(kr/dwkr2);
    // contours
    if(contour_limit<1.0)
    {color = vec4(min(color.xyz, vec3(contour_limit, contour_limit, contour_limit)),1.0);}
    // suggestive contours
    else if((sc_limit<1.0) && dwkr2>dwkr_limit)
    {color = vec4(min(color.xyz, vec3(sc_limit, sc_limit, sc_limit)),1.0);}*/
    /*float kr = abs(t_kr);
    float contour_limit = 20.0*(pow(ndotv, 2.0)/kr);
    //float comb = w.x * dFdx(t_kr) + w.y *dFdy(t_kr) / pow(sqrt(w.x*w.x + w.y*w.y + w.z*w.z),3);
    float comb = dFdx(t_kr) + dFdy(t_kr);
    //if(contour_limit<1.0)
    //{color = vec4(min(color.xyz, vec3(contour_limit, contour_limit, contour_limit)),1.0);}
    if( t_kr <= dwkr_limit && t_kr >= 0 && comb>0){
      color = vec4(0.0, 0.0, 0.0,1.0);
    }
    FragColor = color;
  */

    //FragColor = vec4(LNormal, 1.0);

}
