/*
Bezier Surfaces Mesh class
- Extension of the classic mesh class that supports terrain generation and Bezier Surfaces defined meshes
*/
#pragma once
#include <utils/bezier_surface.h>


class TerrainMesh {
    public:

        std::vector<glm::vec3> m_vertices;
        GLuint VAO, VBO;

        TerrainMesh(const BezierSurface &bsurface)
        {
            m_vertices.reserve(16);
            for (const auto& row : bsurface){
                for (const auto& vertex : row){
                    m_vertices.push_back(glm::vec3(vertex));
                }
            }

            setupMesh();
        }

        TerrainMesh(std::vector<glm::vec3> &new_m_vertices)
        {
            m_vertices.clear(); 
            std::copy(std::begin(new_m_vertices), std::end(new_m_vertices), std::begin(m_vertices));
        }

        // destructor
        ~TerrainMesh() noexcept
        {
            // calls the function which will delete (if needed) the GPU resources
            freeGPUresources();
        }
        

        void Draw() const
        {
            // draw mesh
            glBindVertexArray(VAO);
            glDrawArrays(GL_PATCHES, 0, 16);
            glBindVertexArray(0);
        }   

    private:

        
        void setupMesh()
        {
            // create buffers/arrays
            glGenVertexArrays(1, &VAO);
            glGenBuffers(1, &VBO);
            glBindVertexArray(VAO);
            // load data into vertex buffers
            glBindBuffer(GL_ARRAY_BUFFER, VBO);
            glBufferData(GL_ARRAY_BUFFER, m_vertices.size() * sizeof(glm::vec3), &m_vertices[0], GL_STATIC_DRAW);
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
            glBindVertexArray(0);
        }

        void freeGPUresources()
        {
            // If VAO is 0, this instance of Mesh has been through a move, and no longer owns GPU resources,
            // so there's no need for deleting.
            if (VAO)
            {
                glDeleteVertexArrays(1, &this->VAO);
                glDeleteBuffers(1, &this->VBO);
            }
        }


};





