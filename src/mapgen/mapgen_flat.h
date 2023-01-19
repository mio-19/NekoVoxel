/*
Minetest
Copyright (C) 2015-2020 paramat
Copyright (C) 2015-2016 kwolekr, Ryan Kwolek <kwolekr@minetest.net>

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

/////// Mapgen Flat flags
#define MGFLAT_LAKES 0x01
#define MGFLAT_HILLS 0x02
#define MGFLAT_CAVERNS 0x04

class BiomeManager;

extern FlagDesc flagdesc_mapgen_flat[];

struct MapgenFlatParams : public MapgenParams
{
	s16 ground_level = 8;
	float lake_threshold = -0.45f;
	float lake_steepness = 48.0f;
	float hill_threshold = 0.45f;
	float hill_steepness = 64.0f;

	float cave_width = 0.09f;
	u16 small_cave_num_min = 0;
	u16 small_cave_num_max = 0;
	u16 large_cave_num_min = 0;
	u16 large_cave_num_max = 2;
	s16 large_cave_depth = -33;
	float large_cave_flooded = 0.5f;
	s16 cavern_limit = -256;
	s16 cavern_taper = 256;
	float cavern_threshold = 0.7f;
	s16 dungeon_ymin = -31000;
	s16 dungeon_ymax = 31000;

	NoiseParams np_terrain;
	NoiseParams np_filler_depth;
	NoiseParams np_cavern;
	NoiseParams np_cave1;
	NoiseParams np_cave2;
	NoiseParams np_dungeons;

	MapgenFlatParams();
	~MapgenFlatParams() = default;

	void readParams(const Settings *settings);
	void writeParams(Settings *settings) const;
	void setDefaultSettings(Settings *settings);
};

class MapgenFlat : public MapgenBasic
{
public:
	MapgenFlat(MapgenFlatParams *params, EmergeParams *emerge);
	~MapgenFlat();

	virtual MapgenType getType() const { return MAPGEN_FLAT; }

	virtual void makeChunk(BlockMakeData *data);
	int getSpawnLevelAtPoint(v2size p);
	s16 generateTerrain();

private:
	s16 ground_level;
	float lake_threshold;
	float lake_steepness;
	float hill_threshold;
	float hill_steepness;

	Noise *noise_terrain;
};
