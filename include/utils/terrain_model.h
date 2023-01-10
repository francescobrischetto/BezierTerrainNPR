/*
Bezier Surfaces Model class
- Extension of the classic model class that supports terrain generation and Bezier Surfaces defined meshes
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
    
    //Bezier Surfaces Model created from generation with all the utils classes (Perlin Noise, Terrain Generation, ecc.)
    TerrainModel(unsigned int n, std::int32_t seed, std::int32_t octaves, float freq)
    {
        vector<BezierSurface> terrain_surfaces = gen_Terrain(n, seed, octaves, freq);
        std::vector<TerrainMesh> tmesh;
	    tmesh.reserve(terrain_surfaces.size());
        for (const auto& bsurface : terrain_surfaces)
		    tmesh.emplace_back(bsurface);
        meshes = std::move(tmesh);
        
    }

    //Bezier Surfaces Model created from reading it in memory
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
