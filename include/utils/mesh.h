/*
Mesh class
- the class allocates and initializes VBO, VAO, and EBO buffers, and it sets as OpenGL must consider the data in the buffers

VBO : Vertex Buffer Object - memory allocated on GPU memory to store the mesh data (vertices and their attributes, like e.g. normals, etc)
EBO : Element Buffer Object - a buffer maintaining the indices of vertices composing the mesh faces
VAO : Vertex Array Object - a buffer that helps to "manage" VBO and its inner structure. It stores pointers to the different vertex attributes 
stored in the VBO. When we need to render an object, we can just bind the corresponding VAO, and all the needed calls to set up the binding between vertex attributes and memory positions in the VBO are automatically configured.
*/

#pragma once

using namespace std;

// Std. Includes
#include <vector>

// data structure for vertices
struct Vertex {
    // vertex coordinates
    glm::vec3 Position;
    // Normal
    glm::vec3 Normal;
};

/////////////////// MESH class ///////////////////////
class Mesh {
public:
    // data structures for vertices, and indices of vertices (for faces)
    vector<Vertex> vertices;
    vector<GLuint> indices;
    // VAO
    GLuint VAO;

    // We want Mesh to be a move-only class. We delete copy constructor and copy assignment
    Mesh(const Mesh& copy) = delete; //anzichè delete
    Mesh& operator=(const Mesh &) = delete; //anzichè delete

    // Constructor
    // We use initializer list and std::move in order to avoid a copy of the arguments
    // This constructor empties the source vectors (vertices and indices)
    Mesh(vector<Vertex>& vertices, vector<GLuint>& indices) noexcept
        : vertices(std::move(vertices)), indices(std::move(indices))
    {
        this->setupMesh();
    }

    // We implement a user-defined move constructor and move assignment
    // Move constructor
    // The source object of a move constructor is not expected to be valid after the move.
    // In our case it will no longer imply ownership of the GPU resources and its vectors will be empty.
    Mesh(Mesh&& move) noexcept
        // Calls move for both vectors, which internally consists of a simple pointer swap between the new instance and the source one.
        : vertices(std::move(move.vertices)), indices(std::move(move.indices)),
        VAO(move.VAO), VBO(move.VBO), EBO(move.EBO)
    {
        move.VAO = 0; // We *could* set VBO and EBO to 0 too,
        // but since we bring all the 3 values around we can use just one of them to check ownership of the 3 resources.
    }

    // Move assignment
    Mesh& operator=(Mesh&& move) noexcept
    {
        // calls the function which will delete (if needed) the GPU resources for this instance
        freeGPUresources();

        if (move.VAO) // source instance has GPU resources
        {
            vertices = std::move(move.vertices);
            indices = std::move(move.indices);
            VAO = move.VAO;
            VBO = move.VBO;
            EBO = move.EBO;

            move.VAO = 0;
        }
        else // source instance was already invalid
        {
            VAO = 0;
        }
        return *this;
    }

    // destructor
    ~Mesh() noexcept
    {
        // calls the function which will delete (if needed) the GPU resources
        freeGPUresources();
    }

    //////////////////////////////////////////

    // rendering of mesh
    void Draw(bool isPatches = false)
    {
        // VAO is made "active"
        glBindVertexArray(this->VAO);
        // rendering of data in the VAO
        if(isPatches){
            //This is used For Tessellation Shaders that uses GL_PATCHES instead of GL_TRIANGLES
            glDrawElements(GL_PATCHES, this->indices.size(), GL_UNSIGNED_INT, 0);
        }else{
            glDrawElements(GL_TRIANGLES, this->indices.size(), GL_UNSIGNED_INT, 0);
        }
        // VAO is "detached"
        glBindVertexArray(0);
    }

private:

    // VBO and EBO
    GLuint VBO, EBO;

    //////////////////////////////////////////
    // buffer objects\arrays are initialized
    void setupMesh()
    {
        // we create the buffers
        glGenVertexArrays(1, &this->VAO);
        glGenBuffers(1, &this->VBO);
        glGenBuffers(1, &this->EBO);

        // VAO is made "active"
        glBindVertexArray(this->VAO);
        // we copy data in the VBO - we must set the data dimension, and the pointer to the structure cointaining the data
        glBindBuffer(GL_ARRAY_BUFFER, this->VBO);
        glBufferData(GL_ARRAY_BUFFER, this->vertices.size() * sizeof(Vertex), &this->vertices[0], GL_STATIC_DRAW);
        // we copy data in the EBO - we must set the data dimension, and the pointer to the structure cointaining the data
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, this->indices.size() * sizeof(GLuint), &this->indices[0], GL_STATIC_DRAW);

        // we set in the VAO the pointers to the different vertex attributes (with the relative offsets inside the data structure)
        // vertex positions
        // these will be the positions to use in the layout qualifiers in the shaders ("layout (location = ...)"")
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)0);
        // Normals
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, Normal));
        glBindVertexArray(0);
    }

    //////////////////////////////////////////

    void freeGPUresources()
    {
        // If VAO is 0, this instance of Mesh has been through a move, and no longer owns GPU resources,
        // so there's no need for deleting.
        if (VAO)
        {
            glDeleteVertexArrays(1, &this->VAO);
            glDeleteBuffers(1, &this->VBO);
            glDeleteBuffers(1, &this->EBO);
        }
    }
};
