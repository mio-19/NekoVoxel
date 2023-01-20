/*
Minetest
Copyright (C) 2017 nerzhul, Loic Blot <loic.blot@unix-experience.fr>

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

#include <string>
#include "irrlichttypes_bloated.h"

struct ParticleParameters;
struct ParticleSpawnerParameters;
struct SkyboxParams;
struct SunParams;
struct MoonParams;
struct StarParams;

enum ClientEventType : u8
{
	CE_NONE,
	CE_PLAYER_DAMAGE,
	CE_PLAYER_FORCE_MOVE,
	CE_DEATHSCREEN,
	CE_SHOW_FORMSPEC,
	CE_SHOW_LOCAL_FORMSPEC,
	CE_SPAWN_PARTICLE,
	CE_ADD_PARTICLESPAWNER,
	CE_DELETE_PARTICLESPAWNER,
	CE_HUDADD,
	CE_HUDRM,
	CE_HUDCHANGE,
	CE_SET_SKY,
	CE_SET_SUN,
	CE_SET_MOON,
	CE_SET_STARS,
	CE_OVERRIDE_DAY_NIGHT_RATIO,
	CE_CLOUD_PARAMS,
	CLIENTEVENT_MAX,
};

struct ClientEventHudAdd
{
	u32 server_id;
	u8 type;
	v2f pos, scale;
	std::string name;
	std::string text, text2;
	u32 number, item, dir, style;
	v2f align, offset;
	v3d world_pos;
	v2s32 size;
	s32 z_index;
};

struct ClientEventHudChange
{
	u32 id;
	HudElementStat stat;
	v2f v2fdata;
	std::string sdata;
	u32 data;
	v3d v3fdata;
	v2s32 v2s32data;
};

struct ClientEvent
{
	ClientEventType type;
	union
	{
		// struct{
		//} none;
		struct
		{
			u16 amount;
			bool effect;
		} player_damage;
		struct
		{
			f64 pitch;
			f64 yaw;
		} player_force_move;
		struct
		{
			bool set_camera_point_target;
			f64 camera_point_target_x;
			f64 camera_point_target_y;
			f64 camera_point_target_z;
		} deathscreen;
		struct
		{
			std::string *formspec;
			std::string *formname;
		} show_formspec;
		// struct{
		//} textures_updated;
		ParticleParameters *spawn_particle;
		struct
		{
			ParticleSpawnerParameters *p;
			u16 attached_id;
			u64 id;
		} add_particlespawner;
		struct
		{
			u32 id;
		} delete_particlespawner;
		ClientEventHudAdd *hudadd;
		struct
		{
			u32 id;
		} hudrm;
		ClientEventHudChange *hudchange;
		SkyboxParams *set_sky;
		struct
		{
			bool do_override;
			float ratio_f;
		} override_day_night_ratio;
		struct
		{
			f64 density;
			u32 color_bright;
			u32 color_ambient;
			f64 height;
			f64 thickness;
			f64 speed_x;
			f64 speed_y;
		} cloud_params;
		SunParams *sun_params;
		MoonParams *moon_params;
		StarParams *star_params;
	};
};
