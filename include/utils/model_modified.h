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

// Assimp includes
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <trimesh2/TriMesh.h>

// we include the Mesh class, which manages the "OpenGL side" (= creation and allocation of VBO, VAO, EBO buffers) of the loading of models
#include <utils/mesh_v1.h>

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

    //////////////////////////////////////////

    // model rendering: calls rendering methods of each instance of Mesh class in the vector
    void Draw()
    {
        for(GLuint i = 0; i < this->meshes.size(); i++)
            this->meshes[i].Draw();
    }

    //////////////////////////////////////////


private:

    //////////////////////////////////////////
    // loading of the model using Assimp library. Nodes are processed to build a vector of Mesh class instances
    void loadModel(string path)
    {
        // loading using Assimp
        // N.B.: it is possible to set, if needed, some operations to be performed by Assimp after the loading.
        // Details on the different flags to use are available at: http://assimp.sourceforge.net/lib_html/postprocess_8h.html#a64795260b95f5a4b3f3dc1be4f52e410
        // VERY IMPORTANT: calculation of Tangents and Bitangents is possible only if the model has Texture Coordinates
        // If they are not present, the calculation is skipped (but no error is provided in the following checks!)
        Assimp::Importer importer;
        const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_JoinIdenticalVertices | aiProcess_FlipUVs | aiProcess_GenSmoothNormals | aiProcess_CalcTangentSpace);

        // check for errors (see comment above)
        if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) // if is Not Zero
        {
            cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << endl;
            return;
        }

        // we start the recursive processing of nodes in the Assimp data structure
        this->processNode(scene->mRootNode, scene, path);  
    }

    //////////////////////////////////////////

    // Recursive processing of nodes of Assimp data structure
    void processNode(aiNode* node, const aiScene* scene, const string path)
    {
        // we process each mesh inside the current node
        for(GLuint i = 0; i < node->mNumMeshes; i++)
        {
            std::cout << i <<endl;
            // the "node" object contains only the indices to objects in the scene
            // "Scene" contains all the data. Class node is used only to point to one or more mesh inside the scene and to maintain informations on relations between nodes
            aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
            // we start processing of the Assimp mesh using processMesh method.
            // the result (an istance of the Mesh class) is added to the vector
            // we use emplace_back instead as push_back, so to have the instance created directly in the 
            // vector memory, without the creation of a temp copy.
            // https://en.cppreference.com/w/cpp/container/vector/emplace_back 
            //Mesh processedMesh = processMesh(mesh);
            /*Mesh postProcessedMesh = processMeshTrimesh(&processedMesh, path);*/
            this->meshes.emplace_back(processMesh(mesh, path));
        }
        // we then recursively process each of the children nodes
        for(GLuint i = 0; i < node->mNumChildren; i++)
        {
            this->processNode(node->mChildren[i], scene, path);
        }
    }

    /*Mesh processMeshTrimesh(Mesh* mesh, const string path){
        vector<Vertex> vertices;
        vector<GLuint> indices;
        trimesh::TriMesh *m_triMesh =  trimesh::TriMesh::read( path ) ;
        m_triMesh->normals.clear();
        m_triMesh->need_normals();
        m_triMesh->need_tstrips();
        m_triMesh->need_bbox();
        m_triMesh->need_curvatures();
        m_triMesh->need_dcurv();
        m_triMesh->need_neighbors();

        for(int i=0; i<mesh->vertices.size(); i++){
            Vertex vertex;
            vertex = mesh->vertices[i];
            vertices.push_back(vertex);
        }

        for(int i=0; i<mesh->indices.size(); i++)
            indices.push_back(mesh->indices[i]);

        for( int i = 0; i < m_triMesh->vertices.size(); i++ )
        {   
            if(i==0){
                std::cout << m_triMesh->curv1[i];
            }
            vertices[i].curv1 = m_triMesh->curv1[i];
            vertices[i].curv2 = m_triMesh->curv2[i]; 

            vertices[i].pdir1.x = m_triMesh->pdir1[ i ].x; 
            vertices[i].pdir1.y = m_triMesh->pdir1[ i ].y; 
            vertices[i].pdir1.z = m_triMesh->pdir1[ i ].z; 

            vertices[i].pdir2.x = m_triMesh->pdir2[ i ].x; 
            vertices[i].pdir2.y = m_triMesh->pdir2[ i ].y; 
            vertices[i].pdir2.z = m_triMesh->pdir2[ i ].z; 

            vertices[i].dcurv.x = m_triMesh->dcurv[ i ].x; 
            vertices[i].dcurv.y = m_triMesh->dcurv[ i ].y; 
            vertices[i].dcurv.z = m_triMesh->dcurv[ i ].z; 
            vertices[i].dcurv.w = m_triMesh->dcurv[ i ].w; 
        }
        return Mesh(vertices, indices);
    }*/

    //////////////////////////////////////////

    // Processing of the Assimp mesh in order to obtain an "OpenGL mesh"
    // = we create and allocate the buffers used to send mesh data to the GPU
    Mesh processMesh(aiMesh* mesh, const string path)
    {
        // data structures for vertices and indices of vertices (for faces)
        vector<Vertex> vertices;
        vector<Vertex> vertices2;
        vector<GLuint> indices;
        
        trimesh::TriMesh *m_triMesh =  trimesh::TriMesh::read( path ) ;
        m_triMesh->normals.clear();
        m_triMesh->need_normals();
        m_triMesh->need_curvatures();
        m_triMesh->need_dcurv();

        std::cout<< m_triMesh->vertices.size() <<endl;
        std::cout<< mesh->mNumVertices <<endl;
        for(GLuint i = 0; i < mesh->mNumVertices; i++)
        {
            Vertex vertex;
            // the vector data type used by Assimp is different than the GLM vector needed to allocate the OpenGL buffers
            // I need to convert the data structures (from Assimp to GLM, which are fully compatible to the OpenGL)
            glm::vec3 vector;
            // vertices coordinates
            vector.x = mesh->mVertices[i].x;
            vector.y = mesh->mVertices[i].y;
            vector.z = mesh->mVertices[i].z;
            vertex.Position = vector;
            // Normals
            vector.x = mesh->mNormals[i].x;
            vector.y = mesh->mNormals[i].y;
            vector.z = mesh->mNormals[i].z;
            vertex.Normal = vector;
            // Smoothed Surface Normals, we initialize it with zero vector, and then we refine it.
            vertex.Sm_Normal = glm::vec3();
            // Texture Coordinates
            // if the model has texture coordinates, than we assign them to a GLM data structure, otherwise we set them at 0
            // if texture coordinates are present, than Assimp can calculate tangents and bitangents, otherwise we set them at 0 too
            if(mesh->mTextureCoords[0])
            {
                glm::vec2 vec;
                // in this example we assume the model has only one set of texture coordinates. Actually, a vertex can have up to 8 different texture coordinates. For other models and formats, this code needs to be adapted and modified.
                vec.x = mesh->mTextureCoords[0][i].x;
                vec.y = mesh->mTextureCoords[0][i].y;
                vertex.TexCoords = vec;

                // Tangents
                vector.x = mesh->mTangents[i].x;
                vector.y = mesh->mTangents[i].y;
                vector.z = mesh->mTangents[i].z;
                vertex.Tangent = vector;
                // Bitangents
                vector.x = mesh->mBitangents[i].x;
                vector.y = mesh->mBitangents[i].y;
                vector.z = mesh->mBitangents[i].z;
                vertex.Bitangent = vector;
            }
            else{
                vertex.TexCoords = glm::vec2(0.0f, 0.0f);
                cout << "WARNING::ASSIMP:: MODEL WITHOUT UV COORDINATES -> TANGENT AND BITANGENT ARE = 0" << endl;
            }

            if(i==0){
                std:: cout << m_triMesh->curv1[i] << endl;
                std:: cout << m_triMesh->curv2[i] << endl;
                std:: cout << m_triMesh->pdir1[i] << endl;
                std:: cout << m_triMesh->pdir2[i] << endl;
                std:: cout << m_triMesh->dcurv[i] << endl;
                std:: cout << i << endl << endl;
                std:: cout << m_triMesh->dcurv.size() << endl;
                std:: cout << m_triMesh->curv1.size() << endl;
                std:: cout << m_triMesh->curv2.size() << endl;
                std:: cout << m_triMesh->pdir1.size() << endl;
                std:: cout << m_triMesh->pdir2.size() << endl;

            } //15339
            glm::float32 app;
            app = m_triMesh->curv1[i];
            vertex.curv1 = app;

            app = m_triMesh->curv2[i];
            vertex.curv2 = app;

            vector.x = m_triMesh->pdir1[ i ].x; 
            vector.y = m_triMesh->pdir1[ i ].y; 
            vector.z = m_triMesh->pdir1[ i ].z; 
            vertex.pdir1 = vector;

            vector.x = m_triMesh->pdir2[ i ].x; 
            vector.y = m_triMesh->pdir2[ i ].y; 
            vector.z = m_triMesh->pdir2[ i ].z; 
            vertex.pdir2 = vector;

            glm::vec4 vector2;
            vector2.x = m_triMesh->dcurv[ i ].x; 
            vector2.y = m_triMesh->dcurv[ i ].y; 
            vector2.z = m_triMesh->dcurv[ i ].z; 
            vector2.w = m_triMesh->dcurv[ i ].w;
            vertex.dcurv = vector2;

            // we add the vertex to the list
            vertices.push_back(vertex);
        }

        // for each face of the mesh, we retrieve the indices of its vertices , and we store them in a vector data structure
        for(GLuint i = 0; i < mesh->mNumFaces; i++)
        {
            aiFace face = mesh->mFaces[i];
            for(GLuint j = 0; j < face.mNumIndices; j++)
                indices.push_back(face.mIndices[j]);
        }

        // To calculate smoothed surface normal I started from here: https://www.reddit.com/r/opengl/comments/6976lc/smoothing_function_for_normals/
        // For each face, I calculate the face normal and I add to the vertices' normal used by the face
        // Finally, I normalize the normal to obtain the smoothed surface normal.
        for ( int i = 0; i < indices.size(); i += 3 )
        {
            // I Compute the face normal using triangleNormal method, that computes normal starting from triangle points
            glm::vec3 faceNormal = glm::triangleNormal(
                                        vertices[indices[i]].Position,
                                        vertices[indices[i+1]].Position,
                                        vertices[indices[i+2]].Position);
            // I add face normal to each of the 3 vertex normal of the face
            for ( int j = 0; j < 3; j++ )
            {
                vertices[indices[i+j]].Sm_Normal += faceNormal;
            }
        }
        for (auto &v : vertices)
        {
            // Normalizing the vectors accumulating face normals to obtain the smoothed surface normal
            // NOTE: Sigma parameter, found in Equation 5 of 4.2.1 chapter of the reference paper 
            // ( to control the quantity of convolution kernel used ) is implicitely 1.
            v.Sm_Normal = glm::normalize(v.Sm_Normal);
        }

        // we return an instance of the Mesh class created using the vertices and faces data structures we have created above.
        return Mesh(vertices, indices);
    }
};
