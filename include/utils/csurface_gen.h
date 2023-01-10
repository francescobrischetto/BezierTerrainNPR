/*
CSurfaces subdivision method into step values
*/
#pragma once
#include <utils/csurface.hpp>

std::vector<CSurface> subdiv_CSurface(const CSurface& c, unsigned int w, unsigned int l)
{
	float w_div = 1.0f / (float)w;
	float l_div = 1.0f / (float)l;

	std::vector<CSurface> s;
	s.reserve(w * l);

	for (auto i = 0; i < l; i++)
	{
		auto v = (float)i * l_div;
		auto v_delta = v + l_div;

		for (auto j = 0; j < w; j++)
		{
			auto u = (float)j * w_div;
			auto u_delta = u + w_div;
			s.emplace_back(c.p(u, v), c.p(u_delta, v), c.p(u_delta, v_delta), c.p(u, v_delta));
		}
	}

	return s;
}






