/*
Model class - Modified
- OBJ models loading using Assimp library
- the class converts data from Assimp data structure to a OpenGL-compatible data structure (Mesh class in mesh_v1.h)
- It is a modification of the model_v1.h, that computes also the smoothed surface normal used in fragment shader.

N.B. 1)  
Model and Mesh classes follow RAII principles (https://en.cppreference.com/w/cpp/language/raii).
Model is a "move-only" class. A move-only class ensures that you always have a 1:1 relationship between the total number of resources being created and the total number of actual instantiations occurring.

N.B. 2) no texturing in this version of the class

N.B. 3) based on https://github.com/JoeyDeVries/LearnOpenGL/blob/master/includes/learnopengl/model.h

authors: Davide Gadia, Michael Marchesan
refined by: Francesco Brischetto  mat. 958022

Real-Time Graphics Programming - a.a. 2020/2021
Master degree in Computer Science
Universita' degli Studi di Milano
*/



#pragma once
using namespace std;


// we include the Mesh class, which manages the "OpenGL side" (= creation and allocation of VBO, VAO, EBO buffers) of the loading of models
#include <utils/terrain_mesh.h>
#include <utils/terrain_gen.h>
#include <utils/bezier_surface.h>
#include <sstream>
#include <string>


/////////////////// MODEL class ///////////////////////
class TerrainModel
{
public:
    // at the end of loading, we will have a vector of Mesh class instances
    vector<TerrainMesh> meshes;

    /////////////////////////////////////////
    
    // constructor
    // to notice that Model class is not strictly following the Rules of 5 
    // https://en.cppreference.com/w/cpp/language/rule_of_three
    // because we are not writing a user-defined destructor.

    TerrainModel(unsigned int n, std::int32_t seed, std::int32_t octaves, float freq)
    {
        vector<BezierSurface> terrain_surfaces = gen_Terrain(n, seed, octaves, freq);
        std::vector<TerrainMesh> tmesh;
	    tmesh.reserve(terrain_surfaces.size());
        for (const auto& bsurface : terrain_surfaces)
		    tmesh.emplace_back(bsurface);
        meshes = std::move(tmesh);
        
    }

    TerrainModel(string path)
    {
        vector<BezierSurface> surfaces = readModel(path);
        std::vector<TerrainMesh> tmesh;
	    tmesh.reserve(surfaces.size());
        for (const auto& bsurface : surfaces)
		    tmesh.emplace_back(bsurface);
        meshes = std::move(tmesh);
        
    }

    TerrainModel(){
    }


    //////////////////////////////////////////

    // model rendering: calls rendering methods of each instance of Mesh class in the vector
    void Draw()
    {
        for (const auto& Mesh : meshes)
			Mesh.Draw();
            
    }

    //////////////////////////////////////////


private:

    vector<BezierSurface> readModel(string path)
    {
        vector<BezierSurface> surfaces;
        std::ifstream infile(path);
        std::string line;
        int count = 0;
        BezierSurface bs;
        while (std::getline(infile, line))
        {
            std::istringstream iss(line);
            //First line and every 4 lines (empty), skip
            if(count == 0 || count == 5){
                if(count == 5){
                    surfaces.push_back(bs);
                }
                count=1;
                continue;
            }
            //Process 4 control points
            float p1_x, p1_y, p1_z;
            float p2_x, p2_y, p2_z;
            float p3_x, p3_y, p3_z;
            float p4_x, p4_y, p4_z;
            if (!(iss >> p1_x >> p1_y >> p1_z
                      >> p2_x >> p2_y >> p2_z
                      >> p3_x >> p3_y >> p3_z
                      >> p4_x >> p4_y >> p4_z)) { break; } // error

            ControlVertices app;
            app[0] = glm::vec3(p1_x,p1_y,p1_z);
            app[1] = glm::vec3(p2_x,p2_y,p2_z);
            app[2] = glm::vec3(p3_x,p3_y,p3_z);
            app[3] = glm::vec3(p4_x,p4_y,p4_z);
            bs[count-1] = app;
            count++;
        }
        return surfaces;
    }
};
