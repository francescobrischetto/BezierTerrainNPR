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


    //////////////////////////////////////////

    // model rendering: calls rendering methods of each instance of Mesh class in the vector
    void Draw()
    {
        for (const auto& Mesh : meshes)
			Mesh.Draw();
            
    }

    //////////////////////////////////////////


private:

};
