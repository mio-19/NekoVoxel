/*
Minetest
Copyright (C) 2017 Loic Blot <loic.blot@unix-experience.fr>

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


#include "lua_api/l_minimap.h"
#include "lua_api/l_internal.h"
#include "common/c_converter.h"
#include "client/client.h"
#include "client/minimap.h"
#include "settings.h"

LuaMinimap::LuaMinimap(Minimap *m) : m_minimap(m)
{
}

void LuaMinimap::create(lua_State *L, Minimap *m)
{
	LuaMinimap *o = new LuaMinimap(m);
	*(void **)(lua_newuserdata(L, sizeof(void *))) = o;
	luaL_getmetatable(L, className);
	lua_setmetatable(L, -2);

	// Keep minimap object stack id
	int minimap_object = lua_gettop(L);

	lua_getglobal(L, "core");
	lua_getfield(L, -1, "ui");
	luaL_checktype(L, -1, LUA_TTABLE);
	int uitable = lua_gettop(L);

	lua_pushvalue(L, minimap_object); // Copy object to top of stack
	lua_setfield(L, uitable, "minimap");
}

int LuaMinimap::l_get_pos(lua_State *L)
{
	LuaMinimap *ref = checkObject<LuaMinimap>(L, 1);
	Minimap *m = getobject(ref);

	push_v3s32(L, m->getPos());
	return 1;
}

int LuaMinimap::l_set_pos(lua_State *L)
{
	LuaMinimap *ref = checkObject<LuaMinimap>(L, 1);
	Minimap *m = getobject(ref);

	m->setPos(read_v3s32(L, 2));
	return 1;
}

int LuaMinimap::l_get_angle(lua_State *L)
{
	LuaMinimap *ref = checkObject<LuaMinimap>(L, 1);
	Minimap *m = getobject(ref);

	lua_pushinteger(L, m->getAngle());
	return 1;
}

int LuaMinimap::l_set_angle(lua_State *L)
{
	LuaMinimap *ref = checkObject<LuaMinimap>(L, 1);
	Minimap *m = getobject(ref);

	m->setAngle(lua_tointeger(L, 2));
	return 1;
}

int LuaMinimap::l_get_mode(lua_State *L)
{
	LuaMinimap *ref = checkObject<LuaMinimap>(L, 1);
	Minimap *m = getobject(ref);

	lua_pushinteger(L, m->getModeIndex());
	return 1;
}

int LuaMinimap::l_set_mode(lua_State *L)
{
	LuaMinimap *ref = checkObject<LuaMinimap>(L, 1);
	Minimap *m = getobject(ref);

	u32 mode = lua_tointeger(L, 2);
	if (mode >= m->getMaxModeIndex())
		return 0;

	m->setModeIndex(mode);
	return 1;
}

int LuaMinimap::l_set_shape(lua_State *L)
{
	LuaMinimap *ref = checkObject<LuaMinimap>(L, 1);
	Minimap *m = getobject(ref);
	if (!lua_isnumber(L, 2))
		return 0;

	m->setMinimapShape((MinimapShape)((int)lua_tonumber(L, 2)));
	return 0;
}

int LuaMinimap::l_get_shape(lua_State *L)
{
	LuaMinimap *ref = checkObject<LuaMinimap>(L, 1);
	Minimap *m = getobject(ref);

	lua_pushnumber(L, (int)m->getMinimapShape());
	return 1;
}

int LuaMinimap::l_show(lua_State *L)
{
	// If minimap is disabled by config, don't show it.
	if (!g_settings->getBool("enable_minimap"))
		return 1;

	Client *client = getClient(L);
	assert(client);

	LuaMinimap *ref = checkObject<LuaMinimap>(L, 1);
	Minimap *m = getobject(ref);

	// This is not very adapted to new minimap mode management. Btw, tried
	// to do something compatible.

	if (m->getModeIndex() == 0 && m->getMaxModeIndex() > 0)
		m->setModeIndex(1);

	client->showMinimap(true);
	return 1;
}

int LuaMinimap::l_hide(lua_State *L)
{
	Client *client = getClient(L);
	assert(client);

	LuaMinimap *ref = checkObject<LuaMinimap>(L, 1);
	Minimap *m = getobject(ref);

	// This is not very adapted to new minimap mode management. Btw, tried
	// to do something compatible.

	if (m->getModeIndex() != 0)
		m->setModeIndex(0);

	client->showMinimap(false);
	return 1;
}

Minimap* LuaMinimap::getobject(LuaMinimap *ref)
{
	return ref->m_minimap;
}

int LuaMinimap::gc_object(lua_State *L) {
	LuaMinimap *o = *(LuaMinimap **)(lua_touserdata(L, 1));
	delete o;
	return 0;
}

void LuaMinimap::Register(lua_State *L)
{
	static const luaL_Reg metamethods[] = {
		{"__gc", gc_object},
		{0, 0}
	};
	registerClass(L, className, methods, metamethods);
}

const char LuaMinimap::className[] = "Minimap";
const luaL_Reg LuaMinimap::methods[] = {
	luamethod(LuaMinimap, show),
	luamethod(LuaMinimap, hide),
	luamethod(LuaMinimap, get_pos),
	luamethod(LuaMinimap, set_pos),
	luamethod(LuaMinimap, get_angle),
	luamethod(LuaMinimap, set_angle),
	luamethod(LuaMinimap, get_mode),
	luamethod(LuaMinimap, set_mode),
	luamethod(LuaMinimap, set_shape),
	luamethod(LuaMinimap, get_shape),
	{0,0}
};
