#pragma once
#include <array>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

//Methods definition
glm::vec3 calc_triangle_normal(const glm::vec3& a, const glm::vec3& b, const glm::vec3& c);
glm::vec3 calc_point_on_line(const glm::vec3& p0, const glm::vec3& p1, float t);
glm::vec3 geometric_centre(const std::vector<glm::vec3>& v);
std::array<unsigned int, 2> find_minmax_texcoord_indices(const std::vector<glm::vec3>& v, const glm::vec3& dir, const glm::vec3& origin);

struct vec3Hash
{
	std::size_t operator()(const glm::vec3& v) const
	{
		return std::hash<float>()(v.x) ^ std::hash<float>()(v.y) ^ std::hash<float>()(v.z);
	}


	bool operator()(const glm::vec3& a, const glm::vec3& b) const
	{
		auto x = abs(a.x - b.x);
		auto y = abs(a.y - b.y);
		auto z = abs(a.z - b.z);

		return (x < 0.00005f) && (y < 0.00005) && (z < 0.00005f);
	}

};

//Methods implementation
glm::vec3 geometric_centre(const std::vector<glm::vec3>& v)
{
	auto mean = glm::vec3(0.0f);
	for (const auto& vertex : v)
		mean += vertex;

	return (mean) / (float)(v.size());
}

glm::vec3 calc_triangle_normal(const glm::vec3& a, const glm::vec3& b, const glm::vec3& c)
{
	auto AB = a - b;
	auto AC = a - c;
	return glm::normalize(glm::cross(AB, AC));
}

 glm::vec3 calc_point_on_line(const glm::vec3& p0, const glm::vec3& p1, float t)
{
	return (p0 + (p1 - p0) * t);
}

std::array<unsigned int, 2> find_minmax_texcoord_indices(const std::vector<glm::vec3>& v, const glm::vec3& dir, const glm::vec3& origin)
{

	auto front = glm::normalize(dir);
	auto world_up = glm::vec3(0.0f);


	if (glm::abs(front.y) > 0.999f)
		if (front.y > 0.0f)
			world_up = glm::vec3(0.0f, 0.0f, -1.0f);
		else
			world_up = glm::vec3(0.0f, 0.0f, 1.0f);
	else
		world_up = glm::vec3(0.0f, 1.0f, 0.0f);



	auto image_matrix = glm::lookAt(front + origin, origin, world_up);



	unsigned int min_index = 0;
	unsigned int max_index = 0;
	unsigned int index = 0;
	auto min_point = glm::vec3(0.0f);
	auto max_point = glm::vec3(0.0f);

	for (const auto& vertex : v)
	{
		auto tv = image_matrix * glm::vec4(vertex, 1.0f);

		if (tv.x <= min_point.x && tv.y <= min_point.y)
		{
			min_point = tv;
			min_index = index;;
		}
		if (tv.x >= max_point.x && tv.y >= max_point.y)
		{
			max_point = tv;
			max_index = index;
		}

		index++;
	}

	return std::array<unsigned int, 2>{min_index, max_index};
}