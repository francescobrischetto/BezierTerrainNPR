#version 410 core

out vec4 FragColor;

in vec2 TexCoords;
in float mean_curv;
in vec3 LNormal;
in vec3 pdir1;
in vec3 pdir2;


void main()
{

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
    //FragColor = vec4(LNormal, 1.0);

}
