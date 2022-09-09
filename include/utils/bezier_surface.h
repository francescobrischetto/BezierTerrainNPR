#pragma once
#include <array>
#include <vector>
#include <glm/glm.hpp>
#include <utils/mesh.h>
#include <utils/rand_float.hpp>
#include <glm/glm.hpp>
#include <glm/gtx/normal.hpp>
#include <trimesh2/TriMesh.h>
#include <utils/PerlinNoise.hpp>
typedef std::array<glm::vec3, 4> ControlVertices;
typedef std::array<ControlVertices, 4> BezierSurface;
typedef std::array<int, 2> ControlVertexIndex;

//Methods definition
glm::vec3 eval_BezierCurve(const glm::vec3 &p0, const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& p3, float t) noexcept;
//glm::vec3 eval_BezierSurface(const BezierSurface& s, float u, float v);		// implicit equation for a Bezier Surface
BezierSurface gen_BezierSurfaceMask(float outer_h, float inner_h) noexcept;
glm::vec3 calc_rand_uv(unsigned int i, unsigned int j, float h);
//std::vector<Mesh> evalBezierList(std::vector<BezierSurface> v,std::int32_t seed, std::int32_t octaves, float freq, const int t_points, const int s_points, float disp_factor);
ControlVertexIndex get_BSurfaceCVI(int e_i, int edge_offset, int i) noexcept;

//Methods implementation
glm::vec3 eval_BezierCurve(const glm::vec3& p0, const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& p3, float t) noexcept
{
	float b0 = (1 - t) * (1 - t) * (1 - t);
	float b1 = 3 * t * (1 - t) * (1 - t);
	float b2 = 3 * t * t * (1 - t);
	float b3 = t * t * t;
	return p0 * b0 + p1 * b1 + p2 * b2 + p3 * b3;
}

/*glm::vec3 eval_BezierSurface(const BezierSurface& s, float u, float v)
{
	ControlVertices pu;
	for (auto i = 0; i != 4; i++)
		pu[i] = eval_BezierCurve(s[i][0], s[i][1], s[i][2], s[i][3], u);

	return eval_BezierCurve(pu[0], pu[1], pu[2], pu[3], v);
}*/

glm::vec3 calc_rand_uv(unsigned int i, unsigned int j, float h)
{

	constexpr float div = 0.25;
	float u_min = (float)j * div;
	float u_max = ((float)j + 1.0f) * div;

	float v_min = (float)i * div;
	float v_max = ((float)i + 1.0f) * div;


	static RNG_float rng;
	return { rng(u_min, u_max), rng(v_min, v_max), h };
}

BezierSurface gen_BezierSurfaceMask(float outer_h, float inner_h) noexcept
{
	BezierSurface mask;


	for (unsigned int i = 0; i != 4; i++)
		for (unsigned int j = 0; j != 4; j++)
		{
			if (i == 0 || i == 3)
				mask[i][j] = calc_rand_uv(i, j, outer_h);

			if (i == 1 || i == 2)
			{
				if (j == 0 || j == 3)
					mask[i][j] = calc_rand_uv(i, j, outer_h);
				else
					mask[i][j] = calc_rand_uv(i, j, inner_h);
			}
		}
	return mask;
}

ControlVertexIndex get_BSurfaceCVI(int e_i, int edge_offset, int i) noexcept
{
	if (e_i == 0)
		return { e_i + edge_offset, i };

	if (e_i == 1)
		return { i, 3 - edge_offset };

	if (e_i == 2)
		return { 3 - edge_offset, i };

	if (e_i == 3)
		return { i, edge_offset };


	ControlVertexIndex v{ -1, -1 };
	return v;
}

//Evaluate list of bezier surfaces into list of meshes, computing curvature and other properties using Trimesh2
/*std::vector<Mesh> evalBezierList(std::vector<BezierSurface> surfaces, std::int32_t seed, std::int32_t octaves, float freq, const int t_points, const int s_points, float disp_factor){
	const siv::PerlinNoise perlin(seed);
	vector<Mesh> meshes;
	for(auto& bsurface : surfaces){
		trimesh::TriMesh *m_triMesh = new trimesh::TriMesh(); 
		m_triMesh->set_verbose(0);
		glm::vec3 bezConstructionPoints[4][4][4];
		vector<vector<glm::vec3>> bezPoints(s_points+1, vector<glm::vec3>(t_points+1, glm::vec3(0,0,0)));
		vector<vector<glm::vec3>> bezNormals(s_points+1, vector<glm::vec3>(t_points+1, glm::vec3(0,0,0)));;

		//Initialize construction points
		for (int i = 0; i < 4; i++)
		{
			for (int j = 0; j < 4; j++)
			{
				//Level 3 construction points
				bezConstructionPoints[i][j][3] = bsurface[i][j];
			}
		}
		//COMPUTING TRIANGLES/NORMALS
		float t = 0.0f;
		//Iterate through t
		for (int i = 0; i <= t_points ; i++)
		{
			float s = 0.0f;
			//Iterate through s
			for (int j = 0 ; j <= s_points ; j++)
			{
				//Compute levels until level 0 to get bezier point
				for (int level = 2; level >= 0; level--)
				{
					for (int u = 0; u <= level; u++)
					{
						for (int v = 0; v <= level; v++)
						{
							bezConstructionPoints[u][v][level] = bezConstructionPoints[u][v][level + 1] * ((1 - s)*(1 - t)) + bezConstructionPoints[u][v + 1][level + 1] * ((1 - s)*t) + bezConstructionPoints[u + 1][v][level + 1] * (s*(1 - t)) + bezConstructionPoints[u + 1][v + 1][level + 1] * (s*t);
						}
					}
				}
				//Add point to bezier array
				bezPoints[i][j] = bezConstructionPoints[0][0][0];
				//Compute difdis from the construction points
				glm::vec3 difdis = (bezConstructionPoints[0][0][3] * (float)pow((1 - t), 3) 
									+ bezConstructionPoints[0][1][3] * (float)3 * t*(float)pow((1 - t), 2)
									+ bezConstructionPoints[0][2][3] * (float)3 * (float)pow(t, 2)*(1 - t) 
									+ bezConstructionPoints[0][3][3] * (float)pow(t, 3))*(-3+6*s-3*(float)pow(s,2)) 
									+(bezConstructionPoints[1][0][3] * (float)pow((1 - t), 3) 
									+ bezConstructionPoints[1][1][3] * (float)3 * t*(float)pow((1 - t), 2)
									+ bezConstructionPoints[1][2][3] * (float)3 * (float)pow(t, 2)*(1 - t) 
									+ bezConstructionPoints[1][3][3] * (float)pow(t, 3)) *(3-12*s+9*(float)pow(s,2)) 
									+(bezConstructionPoints[2][0][3] * (float)pow((1 - t), 3) 
									+ bezConstructionPoints[2][1][3] * (float)3 * t*(float)pow((1 - t), 2)
									+ bezConstructionPoints[2][2][3] * (float)3 * (float)pow(t, 2)*(1 - t) 
									+ bezConstructionPoints[2][3][3] * (float)pow(t, 3)) * (6*s-9*(float)pow(s,2)) 
									+(bezConstructionPoints[3][0][3] * (float)pow((1 - t), 3) 
									+ bezConstructionPoints[3][1][3] * (float)3 * t*(float)pow((1 - t), 2)
									+ bezConstructionPoints[3][2][3] * (float)3 * (float)pow(t, 2)*(1 - t) 
									+ bezConstructionPoints[3][3][3] * (float)pow(t, 3))*(float)3*(float)pow(s,2);


				//Compute difdit from the construction points
				glm::vec3 difdit = (bezConstructionPoints[0][0][3] * (float)pow((1 - s), 3) 
									+ bezConstructionPoints[1][0][3] *(float) 3 * s*(float)pow((1 - s), 2)
									+ bezConstructionPoints[2][0][3] * (float)3 * (float)pow(s, 2)*(1 - s) 
									+ bezConstructionPoints[3][0][3] * (float)pow(s, 3))*(-3 + 6 * t - 3 * (float)pow(t, 2)) 
									+(bezConstructionPoints[0][1][3] * (float)pow((1 - s), 3) 
									+ bezConstructionPoints[1][1][3] * (float)3 * s*(float)pow((1 - s), 2)
									+ bezConstructionPoints[2][1][3] * (float)3 * (float)pow(s, 2)*(1 - s) 
									+ bezConstructionPoints[3][1][3] * (float)pow(s, 3)) *(3 - 12 * t + 9 * (float)pow(t, 2)) 
									+(bezConstructionPoints[0][2][3] * (float)pow((1 - s), 3) 
									+ bezConstructionPoints[1][2][3] * (float)3 * s*(float)pow((1 - s), 2)
									+ bezConstructionPoints[2][2][3] * (float)3 * (float)pow(s, 2)*(1 - s) 
									+ bezConstructionPoints[3][2][3] * (float)pow(s, 3)) * (6 * t - 9 * (float)pow(t,2)) 
									+(bezConstructionPoints[0][3][3] * (float)pow((1 - s), 3) 
									+ bezConstructionPoints[1][3][3] * (float)3 * s*(float)pow((1 - s), 2)
									+ bezConstructionPoints[2][3][3] * (float)3 * (float)pow(s, 2)*(1 - s) 
									+ bezConstructionPoints[3][3][3] * (float)pow(s, 3)) * (float)3 * (float)pow(t, 2);

				//Store the vertex normal
				bezNormals[i][j] = glm::normalize(glm::cross(difdit,difdis));
				
				//displacement factor based on perlin Noise
				if((i>0 && i<t_points) && (j>0 && j<s_points)){
					
					//TODO: FIX THIS? SMOOTHING MESH NORMALS IS NEEDED
					bezPoints[i][j] += disp_factor * (float) perlin.accumulatedOctaveNoise2D_0_1(i / ((t_points*2)  / freq), j / ((s_points*2)  / freq), octaves) * glm::normalize(bezNormals[i][j]);
				}
				//Step s
				s += (1 / (float)s_points);
				m_triMesh->vertices.push_back(trimesh::point3(trimesh::vec3(bezPoints[i][j].x,bezPoints[i][j].y,bezPoints[i][j].z)));
				m_triMesh->normals.push_back(trimesh::point3(trimesh::vec3(bezNormals[i][j].x,bezNormals[i][j].y,bezNormals[i][j].z)));
			}
			//Step t
			t += (1 / (float)t_points);
		}
		//COMPUTING FACES		
		for (int i = 0; i < t_points; i++)
		{
			for (int j = 0; j < s_points; j++)
			{
				//Break each quad to 2 triangles
				m_triMesh->faces.push_back(trimesh::TriMesh::Face(i*(s_points + 1) +j,(i+1)*(s_points + 1) +j,i*(s_points + 1) +j+1));
				m_triMesh->faces.push_back(trimesh::TriMesh::Face((i+1)*(s_points + 1) +j,(i+1)*(s_points + 1) +j+1,i*(s_points + 1) +j+1));
			}
		}
		//Trimesh object is now loaded with vertices, normals and faces
		//m_triMesh->normals.clear();
        //m_triMesh->need_normals();
		m_triMesh->need_curvatures();
        m_triMesh->need_dcurv();

		vector<Vertex> vertices;
        vector<GLuint> indices;
		//Creating Meshes using mesh.h defined class
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
		meshes.emplace_back(Mesh(vertices, indices));
	}
	return meshes;
}*/