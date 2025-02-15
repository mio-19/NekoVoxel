/*
Minetest
Copyright (C) 2017-2019 vlapsley, Vaughan Lapsley <vlapsley@gmail.com>
Copyright (C) 2017-2019 paramat

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

#define MGCARPATHIAN_CAVERNS 0x01
#define MGCARPATHIAN_RIVERS  0x02

class BiomeManager;

extern FlagDesc flagdesc_mapgen_carpathian[];


struct MapgenCarpathianParams : public MapgenParams
{
	float base_level       = 12.0f;
	float river_width      = 0.05f;
	float river_depth      = 24.0f;
	float valley_width     = 0.25f;

	float cave_width         = 0.09f;
	s32 large_cave_depth     = -33;
	u16 small_cave_num_min   = 0;
	u16 small_cave_num_max   = 0;
	u16 large_cave_num_min   = 0;
	u16 large_cave_num_max   = 2;
	float large_cave_flooded = 0.5f;
	s32 cavern_limit         = -256;
	s32 cavern_taper         = 256;
	float cavern_threshold   = 0.7f;
	s32 dungeon_ymin         = -2100000000;
	s32 dungeon_ymax         = 2100000000;

	NoiseParams np_filler_depth;
	NoiseParams np_height1;
	NoiseParams np_height2;
	NoiseParams np_height3;
	NoiseParams np_height4;
	NoiseParams np_hills_terrain;
	NoiseParams np_ridge_terrain;
	NoiseParams np_step_terrain;
	NoiseParams np_hills;
	NoiseParams np_ridge_mnt;
	NoiseParams np_step_mnt;
	NoiseParams np_rivers;
	NoiseParams np_mnt_var;
	NoiseParams np_cave1;
	NoiseParams np_cave2;
	NoiseParams np_cavern;
	NoiseParams np_dungeons;

	MapgenCarpathianParams();
	~MapgenCarpathianParams() = default;

	void readParams(const Settings *settings);
	void writeParams(Settings *settings) const;
	void setDefaultSettings(Settings *settings);
};

class MapgenCarpathian : public MapgenBasic
{
public:
	MapgenCarpathian(MapgenCarpathianParams *params, EmergeParams *emerge);
	~MapgenCarpathian();

	virtual MapgenType getType() const { return MAPGEN_CARPATHIAN; }

	virtual void makeChunk(BlockMakeData *data);
	int getSpawnLevelAtPoint(v2s32 p);

private:
	float base_level;
	float river_width;
	float river_depth;
	float valley_width;

	Noise *noise_height1;
	Noise *noise_height2;
	Noise *noise_height3;
	Noise *noise_height4;
	Noise *noise_hills_terrain;
	Noise *noise_ridge_terrain;
	Noise *noise_step_terrain;
	Noise *noise_hills;
	Noise *noise_ridge_mnt;
	Noise *noise_step_mnt;
	Noise *noise_rivers = nullptr;
	Noise *noise_mnt_var;

	s32 grad_wl;

	float getSteps(float noise);
	inline float getLerp(float noise1, float noise2, float mod);
	int generateTerrain();
};
