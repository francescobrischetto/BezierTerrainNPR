/*
Model class - Modified
- OBJ models loading using Trimesh library
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
    Model(const Model& model) = delete; //disallow copy
    Model& operator=(const Model& copy) = delete;
    
    // For the Model class, a default move constructor and move assignment is sufficient
    Model& operator=(Model&& move) noexcept = default;
    Model(Model&& model) = default; //internally does a memberwise std::move
    
    // constructor
    Model(const string& path)
    {
        this->loadModel(path);
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
        for( int i = 0; i < m_triMesh->vertices.size(); i++ )
	    {
            Vertex vertex;
            glm::vec3 vector;

            vector.x = m_triMesh->vertices[ i ][ 0 ];
            vector.y = m_triMesh->vertices[ i ][ 1 ];
            vector.z = m_triMesh->vertices[ i ][ 2 ];
            vertex.Position = vector;

            vector.x = m_triMesh->normals[i][0];
            vector.y = m_triMesh->normals[i][1];
            vector.z = m_triMesh->normals[i][2];
            vertex.Normal = vector;

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
