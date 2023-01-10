/*
Terrain generation (as list of Bezier Surfaces)
- Other useful methods to achieve this generation
*/
#pragma once
#include <utils/bezier_surface.h>
#include <utils/geometry_util.h>
#include <utils/csurface.hpp>
#include <utils/csurface_gen.h>
#include <utils/PerlinNoise.hpp>

class CSurface;

//Methods definition
void stitch_BezierSurfaces(unsigned int l, unsigned int w, std::vector<BezierSurface>& bsurfaces);
void stitch_ADJEdges_smooth(BezierSurface &b0, BezierSurface &b1, bool horizontal);					
std::vector<BezierSurface> gen_Terrain(unsigned int n, std::int32_t seed, std::int32_t octaves, float freq);
std::vector<BezierSurface> gen_TerrainMasks(unsigned int l, unsigned int w, std::int32_t seed, std::int32_t octaves, float freq);
BezierSurface gen_TerrainSurface(const CSurface& surface, const BezierSurface& mask);
std::vector<BezierSurface> gen_TerrainSurfaces(const std::vector<CSurface> &l, const std::vector<BezierSurface> &masks);

//Methods implementation
BezierSurface gen_TerrainSurface(const CSurface& surface, const BezierSurface& mask)
{
	BezierSurface bsurface;
	for (unsigned int i = 0; i != 4; i++)
		for (unsigned int j = 0; j != 4; j++)
			bsurface[i][j] = surface.p(mask[i][j][0], mask[i][j][1], mask[i][j][2]);

	return bsurface;
}

std::vector<BezierSurface> gen_TerrainSurfaces(const std::vector<CSurface>& l, const std::vector<BezierSurface>& masks)
{
	std::vector<BezierSurface> t_surfaces;
	t_surfaces.reserve(l.size());
	for (auto i = 0; i != l.size(); i++)
		t_surfaces.emplace_back(gen_TerrainSurface(l[i], masks[i]));


	return t_surfaces;
}

//The real methods where all the generations starts
std::vector<BezierSurface> gen_Terrain(unsigned int n, std::int32_t seed, std::int32_t octaves, float freq)
{
	//The space reserved from the generation of the terrain (from -2.0, 0.0, 2.0 to -2.0 0.0 -2.0)
	auto c = CSurface(glm::vec3(-2.0, 0.0, 2.0), glm::vec3(2.0, 0.0, 2.0), glm::vec3(2.0, 0.0, -2.0), glm::vec3(-2.0, 0.0, -2.0));			// XZ PLANE WITH NORMAL (0.0, 1.0, 0.0)
	auto s = subdiv_CSurface(c,n,n);
	auto m = gen_TerrainMasks(n, n, seed, octaves, freq);
	auto t = gen_TerrainSurfaces(s, m);
	stitch_BezierSurfaces(n, n, t);
	return t;
}

std::vector<BezierSurface> gen_TerrainMasks(unsigned int l, unsigned int w, std::int32_t seed, std::int32_t octaves, float freq)
{
	const siv::PerlinNoise perlin(seed);
	const double fx = (w*2)  / freq;
	const double fy = (l*2) / freq;
	std::vector<BezierSurface> masks;
	for (auto y = 0; y != l * 2; y += 2)
	{
		for (auto x = 0; x < w * 2; x += 2)
		{
			auto m = gen_BezierSurfaceMask(0, 0);
			// gen mask with accumulated perlin noise that will change height of all points
			m[0][0].z = 1.3f * perlin.accumulatedOctaveNoise2D_0_1((x + 1.0) / fx, y / fy, octaves);
			m[0][1].z = 1.3f * perlin.accumulatedOctaveNoise2D_0_1(x / fx, y / fy, octaves);
			m[0][2].z = 1.3f * perlin.accumulatedOctaveNoise2D_0_1((x + 1.0) / fx, y / fy, octaves);
			m[0][3].z = 1.3f * perlin.accumulatedOctaveNoise2D_0_1(x / fx, (y + 1.0) / fy, octaves);
			m[1][0].z = 1.3f * perlin.accumulatedOctaveNoise2D_0_1((x + 1.0) / fx, y / fy, octaves);
			m[2][0].z = 1.3f * perlin.accumulatedOctaveNoise2D_0_1(x / fx, y / fy, octaves);
			m[3][0].z = 1.3f * perlin.accumulatedOctaveNoise2D_0_1((x + 1.0) / fx, y / fy, octaves);
			m[3][1].z = 1.3f * perlin.accumulatedOctaveNoise2D_0_1(x / fx, y / fy, octaves);
			m[3][2].z = 1.3f * perlin.accumulatedOctaveNoise2D_0_1((x + 1.0) / fx, y / fy, octaves);
			m[3][3].z = 1.3f * perlin.accumulatedOctaveNoise2D_0_1(x / fx, y / fy, octaves);
			m[1][3].z = 1.3f * perlin.accumulatedOctaveNoise2D_0_1((x + 1.0) / fx, (y + 1.0) / fy, octaves);
			m[2][3].z = 1.3f * perlin.accumulatedOctaveNoise2D_0_1(x / fx, (y + 1.0) / fy, octaves);
			m[1][1].z = 1.3f * perlin.accumulatedOctaveNoise2D_0_1(x / fx, y / fy, octaves);
			m[1][2].z = 1.3f * perlin.accumulatedOctaveNoise2D_0_1((x + 1.0) / fx, y / fy, octaves);
			m[2][1].z = 1.3f * perlin.accumulatedOctaveNoise2D_0_1(x / fx, (y + 1.0) / fy, octaves);
			m[2][2].z = 1.3f * perlin.accumulatedOctaveNoise2D_0_1((x + 1.0) / fx, (y + 1.0) / fy, octaves);
			masks.emplace_back(std::move(m));
		}
	}

	return masks;
}

void stitch_BezierSurfaces(unsigned int l, unsigned int w, std::vector<BezierSurface>& bsurfaces)
{
	if (l * w != bsurfaces.size())
		return;


	// stitch horizontally
	auto b_i = 0;
	for (auto j = 0; j != l; j++)
	{
		for (auto i = b_i; i != (b_i + w) - 1; i++)
			stitch_ADJEdges_smooth(bsurfaces[i], bsurfaces[i + 1], true);
		b_i += w;
	}
	
	// stitch vertically
	b_i = 0;
	for (auto j = 0; j != l - 1; j++)
	{
		for (auto i = b_i; i != b_i + w; i++)
			stitch_ADJEdges_smooth(bsurfaces[i], bsurfaces[i + w], false);
		b_i += w;
	}
	

}

void stitch_ADJEdges_smooth(BezierSurface& b0, BezierSurface& b1, bool horizontal)
{
	auto b0_ei = 2;
	auto b1_ei = 0;

	if (horizontal)
	{
		b0_ei = 1;
		b1_ei = 3;
	}

	for (auto i = 0; i != 4; i++)
	{
		auto p0_vi = get_BSurfaceCVI(b0_ei, 2, i);
		auto p1_vi = get_BSurfaceCVI(b0_ei, 1, i);
		auto p2_vi = get_BSurfaceCVI(b1_ei, 1, i);
		auto p3_vi = get_BSurfaceCVI(b1_ei, 2, i);

		auto p0 = b0[p0_vi[0]][p0_vi[1]];
		auto p1 = b0[p1_vi[0]][p1_vi[1]];
		auto p2 = b1[p2_vi[0]][p2_vi[1]];
		auto p3 = b1[p3_vi[0]][p3_vi[1]];



		auto p0_n = eval_BezierCurve(p0, p1, p2, p3, 0.25);
		auto p3_n = eval_BezierCurve(p0, p1, p2, p3, 0.75);
		auto p12_n = calc_point_on_line(p0_n, p3_n, 0.50);

		auto b0_vi = get_BSurfaceCVI(b0_ei, 0, i);
		auto b1_vi = get_BSurfaceCVI(b1_ei, 0, i);


		b0[p1_vi[0]][p1_vi[1]] = p0_n;
		b0[b0_vi[0]][b0_vi[1]] = p12_n;
		b1[b1_vi[0]][b1_vi[1]] = p12_n;
		b1[p2_vi[0]][p2_vi[1]] = p3_n;

	}
}


