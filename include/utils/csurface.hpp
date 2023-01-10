/*
Continuous Surface Class:
- class to define continuous surfaces (specialized lately in Bezier Surfaces)
*/
#pragma once
#include <glm/glm.hpp>
#include <vector>
#include <utils/geometry_util.h>

// Class for Continuous Surfaces
class CSurface
{
public:

	CSurface(glm::vec3&& bl, glm::vec3&& br, glm::vec3&& tr, glm::vec3&& tl) : points{ bl, br, tr, tl } { n = calc_triangle_normal(points[0], points[1], points[3]); }

	const std::vector<glm::vec3>& vertices() const noexcept { return points; }
	glm::vec3 normal() const noexcept { return n; }
	glm::vec3 p(float u, float v, float h = 0.0f) const { auto p = ((1 - u) * (1 - v) * points[0]) + (u * (1 - v) * points[1]) + ((1 - u) * v * points[3]) + (u * v * points[2]); return p + (n * h); }



private:
	std::vector<glm::vec3> points;
	glm::vec3 n;

};




