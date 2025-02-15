/*
Minetest
Copyright (C) 2010-2018 celeron55, Perttu Ahola <celeron55@gmail.com>
Copyright (C) 2013-2018 kwolekr, Ryan Kwolek <kwolekr@minetest.net>
Copyright (C) 2014-2018 paramat

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation; either version 2.1 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along
with this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#pragma once

#include "mapgen.h"
#include "noise.h"

#define MGV6_AVERAGE_MUD_AMOUNT 4
#define MGV6_DESERT_STONE_BASE -32
#define MGV6_ICE_BASE 0
#define MGV6_FREQ_HOT 0.4
#define MGV6_FREQ_SNOW -0.4
#define MGV6_FREQ_TAIGA 0.5
#define MGV6_FREQ_JUNGLE 0.5

//////////// Mapgen V6 flags
#define MGV6_JUNGLES    0x01
#define MGV6_BIOMEBLEND 0x02
#define MGV6_MUDFLOW    0x04
#define MGV6_SNOWBIOMES 0x08
#define MGV6_FLAT       0x10
#define MGV6_TREES      0x20


extern FlagDesc flagdesc_mapgen_v6[];


enum BiomeV6Type
{
	BT_NORMAL,
	BT_DESERT,
	BT_JUNGLE,
	BT_TUNDRA,
	BT_TAIGA,
};


struct MapgenV6Params : public MapgenParams {
	float freq_desert = 0.45f;
	float freq_beach = 0.15f;
	s32 dungeon_ymin = -2100000000;
	s32 dungeon_ymax = 2100000000;

	NoiseParams np_terrain_base;
	NoiseParams np_terrain_higher;
	NoiseParams np_steepness;
	NoiseParams np_height_select;
	NoiseParams np_mud;
	NoiseParams np_beach;
	NoiseParams np_biome;
	NoiseParams np_cave;
	NoiseParams np_humidity;
	NoiseParams np_trees;
	NoiseParams np_apple_trees;

	MapgenV6Params();
	~MapgenV6Params() = default;

	void readParams(const Settings *settings);
	void writeParams(Settings *settings) const;
	void setDefaultSettings(Settings *settings);
};


class MapgenV6 : public Mapgen {
public:
	EmergeParams *m_emerge;

	int ystride;
	u32 spflags;

	v3s32 node_min;
	v3s32 node_max;
	v3s32 full_node_min;
	v3s32 full_node_max;
	v3s32 central_area_size;

	Noise *noise_terrain_base;
	Noise *noise_terrain_higher;
	Noise *noise_steepness;
	Noise *noise_height_select;
	Noise *noise_mud;
	Noise *noise_beach;
	Noise *noise_biome;
	Noise *noise_humidity;
	NoiseParams *np_cave;
	NoiseParams *np_humidity;
	NoiseParams *np_trees;
	NoiseParams *np_apple_trees;

	NoiseParams np_dungeons;

	float freq_desert;
	float freq_beach;
	s32 dungeon_ymin;
	s32 dungeon_ymax;

	content_t c_stone;
	content_t c_dirt;
	content_t c_dirt_with_grass;
	content_t c_sand;
	content_t c_water_source;
	content_t c_lava_source;
	content_t c_gravel;
	content_t c_desert_stone;
	content_t c_desert_sand;
	content_t c_dirt_with_snow;
	content_t c_snow;
	content_t c_snowblock;
	content_t c_ice;

	content_t c_cobble;
	content_t c_mossycobble;
	content_t c_stair_cobble;
	content_t c_stair_desert_stone;

	MapgenV6(MapgenV6Params *params, EmergeParams *emerge);
	~MapgenV6();

	virtual MapgenType getType() const { return MAPGEN_V6; }

	void makeChunk(BlockMakeData *data);
	int getGroundLevelAtPoint(v2s32 p);
	int getSpawnLevelAtPoint(v2s32 p);

	float baseTerrainLevel(float terrain_base, float terrain_higher,
		float steepness, float height_select);
	virtual float baseTerrainLevelFromNoise(v2s32 p);
	virtual float baseTerrainLevelFromMap(v2s32 p);
	virtual float baseTerrainLevelFromMap(int index);

	s32 find_stone_level(v2s32 p2d);
	bool block_is_underground(u64 seed, v3s32 blockpos);

	float getHumidity(v2s32 p);
	float getTreeAmount(v2s32 p);
	bool getHaveAppleTree(v2s32 p);
	float getMudAmount(int index);
	bool getHaveBeach(int index);
	BiomeV6Type getBiome(v2s32 p);
	BiomeV6Type getBiome(int index, v2s32 p);

	u32 get_blockseed(u64 seed, v3s32 p);

	virtual void calculateNoise();
	int generateGround();
	void addMud();
	void flowMud(s32 &mudflow_minpos, s32 &mudflow_maxpos);
	void moveMud(u32 remove_index, u32 place_index,
		u32 above_remove_index, v2s32 pos, v3s32 em);
	void growGrass();
	void placeTreesAndJungleGrass();
	virtual void generateCaves(int max_stone_y);
};
