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

// we use GLM data structures to convert data in the Assimp data structures in a data structures suited for VBO, VAO and EBO buffers
#include <glm/glm.hpp>
#include <glm/gtx/normal.hpp>
#include <trimesh2/TriMesh.h>

// we include the Mesh class, which manages the "OpenGL side" (= creation and allocation of VBO, VAO, EBO buffers) of the loading of models
#include <utils/mesh.h>
#include <utils/terrain_gen.h>
#include <utils/bezier_surface.h>


/////////////////// MODEL class ///////////////////////
class Model
{
public:
    // at the end of loading, we will have a vector of Mesh class instances
    vector<Mesh> meshes;

    //////////////////////////////////////////

    // We want Model to be a move-only class. We delete copy constructor and copy assignment
    // see:
    // https://docs.microsoft.com/en-us/cpp/cpp/constructors-cpp?view=vs-2019
    // https://en.cppreference.com/w/cpp/language/copy_constructor
    // https://en.cppreference.com/w/cpp/language/copy_assignment
    // https://www.geeksforgeeks.org/preventing-object-copy-in-cpp-3-different-ways/
    // Section 4.6 of the "A Tour in C++" book
    Model(const Model& model) = delete; //disallow copy
    Model& operator=(const Model& copy) = delete;
    
    // For the Model class, a default move constructor and move assignment is sufficient
    // see:
    // https://docs.microsoft.com/en-us/cpp/cpp/move-constructors-and-move-assignment-operators-cpp?view=vs-2019
    // https://en.cppreference.com/w/cpp/language/move_constructor
    // https://en.cppreference.com/w/cpp/language/move_assignment
    // https://www.learncpp.com/cpp-tutorial/15-1-intro-to-smart-pointers-move-semantics/
    // https://www.learncpp.com/cpp-tutorial/15-3-move-constructors-and-move-assignment/
    // Section 4.6 of the "A Tour in C++" book
    Model& operator=(Model&& move) noexcept = default;
    Model(Model&& model) = default; //internally does a memberwise std::move
    
    // constructor
    // to notice that Model class is not strictly following the Rules of 5 
    // https://en.cppreference.com/w/cpp/language/rule_of_three
    // because we are not writing a user-defined destructor.
    Model(const string& path)
    {
        this->loadModel(path);
    }

    Model(unsigned int n, std::int32_t seed, std::int32_t octaves, float freq, const unsigned int t_points, const unsigned int s_points, float disp_factor)
    {
        vector<BezierSurface> terrain_surfaces = gen_Terrain(n, seed, octaves, freq);
        vector<Mesh> meshes = evalBezierList(terrain_surfaces,seed,octaves,freq, t_points, s_points, disp_factor);
        for(auto& mesh : meshes){
            this->meshes.emplace_back(Mesh(mesh.vertices,mesh.indices));
        }
    }


    //////////////////////////////////////////

    // model rendering: calls rendering methods of each instance of Mesh class in the vector
    void Draw(bool isPatches = false)
    {
        for(GLuint i = 0; i < this->meshes.size(); i++)
            this->meshes[i].Draw(isPatches);
    }

    //////////////////////////////////////////


private:

    //////////////////////////////////////////
    // loading of the model using Trimesh library. Nodes are processed to build a vector of Mesh class instances
    void loadModel(string path)
    {
        trimesh::TriMesh *m_triMesh ;
        m_triMesh->set_verbose(0);
        m_triMesh =  trimesh::TriMesh::read( path ) ;
        vector<Vertex> vertices;
        vector<GLuint> indices;
        m_triMesh->normals.clear();
        m_triMesh->need_normals();
        m_triMesh->need_curvatures();
        m_triMesh->need_dcurv();
        for( int i = 0; i < m_triMesh->vertices.size(); i++ )
	    {
            Vertex vertex;
            glm::vec3 vector;
            glm::vec4 vector2;
            glm::float32 app;

            vector.x = m_triMesh->vertices[ i ][ 0 ];
            vector.y = m_triMesh->vertices[ i ][ 1 ];
            vector.z = m_triMesh->vertices[ i ][ 2 ];
            vertex.Position = vector;

            vector.x = m_triMesh->normals[i][0];
            vector.y = m_triMesh->normals[i][1];
            vector.z = m_triMesh->normals[i][2];
            vertex.Normal = vector;

            
            app = m_triMesh->curv1[i];
            vertex.curv1 = app;

            app = m_triMesh->curv2[i];
            vertex.curv2 = app;

            app = m_triMesh->feature_size();
            vertex.fz = app;

            vector.x = m_triMesh->pdir1[ i ].x; 
            vector.y = m_triMesh->pdir1[ i ].y; 
            vector.z = m_triMesh->pdir1[ i ].z; 
            vertex.pdir1 = vector;

            vector.x = m_triMesh->pdir2[ i ].x; 
            vector.y = m_triMesh->pdir2[ i ].y; 
            vector.z = m_triMesh->pdir2[ i ].z; 
            vertex.pdir2 = vector;
    
            vector2.x = m_triMesh->dcurv[ i ].x; 
            vector2.y = m_triMesh->dcurv[ i ].y; 
            vector2.z = m_triMesh->dcurv[ i ].z; 
            vector2.w = m_triMesh->dcurv[ i ].w;
            vertex.dcurv = vector2;

            // we add the vertex to the list
            vertices.push_back(vertex);
        }

        // for each face of the mesh, we retrieve the indices of its vertices , and we store them in a vector data structure
        for(GLuint i = 0; i < m_triMesh->faces.size(); i++)
        {
            indices.push_back(m_triMesh->faces[i][0]);
            indices.push_back(m_triMesh->faces[i][1]);
            indices.push_back(m_triMesh->faces[i][2]);
        }

        // we return an instance of the Mesh class created using the vertices and faces data structures we have created above.
        this->meshes.emplace_back(Mesh(vertices, indices));


    }
    //////////////////////////////////////////
};
