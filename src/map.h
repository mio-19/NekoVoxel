/*
Minetest
Copyright (C) 2010-2013 celeron55, Perttu Ahola <celeron55@gmail.com>

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

#include <iostream>
#include <sstream>
#include <set>
#include <map>
#include <list>

#include "irrlichttypes_bloated.h"
#include "mapblock.h"
#include "mapnode.h"
#include "constants.h"
#include "voxel.h"
#include "modifiedstate.h"
#include "util/container.h"
#include "util/metricsbackend.h"
#include "util/numeric.h"
#include "nodetimer.h"
#include "map_settings_manager.h"
#include "debug.h"

class Settings;
class MapDatabase;
class ClientMap;
class MapSector;
class ServerMapSector;
class MapBlock;
class NodeMetadata;
class IGameDef;
class IRollbackManager;
class EmergeManager;
class MetricsBackend;
class ServerEnvironment;
struct BlockMakeData;

/*
	MapEditEvent
*/

enum MapEditEventType{
	// Node added (changed from air or something else to something)
	MEET_ADDNODE,
	// Node removed (changed to air)
	MEET_REMOVENODE,
	// Node swapped (changed without metadata change)
	MEET_SWAPNODE,
	// Node metadata changed
	MEET_BLOCK_NODE_METADATA_CHANGED,
	// Anything else (modified_blocks are set unsent)
	MEET_OTHER
};

struct MapEditEvent
{
	MapEditEventType type = MEET_OTHER;
	v3size p;
	MapNode n = CONTENT_AIR;
	std::vector<v3size> modified_blocks; // Represents a set
	bool is_private_change = false;

	MapEditEvent() = default;

	// Sets the event's position and marks the block as modified.
	void setPositionModified(v3size pos)
	{
		assert(modified_blocks.empty()); // only meant for initialization (once)
		p = pos;
		modified_blocks.push_back(getNodeBlockPos(pos));
	}

	void setModifiedBlocks(const std::map<v3size, MapBlock *> blocks)
	{
		assert(modified_blocks.empty()); // only meant for initialization (once)
		modified_blocks.reserve(blocks.size());
		for (const auto &block : blocks)
			modified_blocks.push_back(block.first);
	}

	VoxelArea getArea() const
	{
		switch(type){
		case MEET_ADDNODE:
		case MEET_REMOVENODE:
		case MEET_SWAPNODE:
		case MEET_BLOCK_NODE_METADATA_CHANGED:
			return VoxelArea(p);
		case MEET_OTHER:
		{
			VoxelArea a;
			for (v3size p : modified_blocks) {
				v3size np1 = p*MAP_BLOCKSIZE;
				v3size np2 = np1 + v3size(1,1,1)*MAP_BLOCKSIZE - v3size(1,1,1);
				a.addPoint(np1);
				a.addPoint(np2);
			}
			return a;
		}
		}
		return VoxelArea();
	}
};

class MapEventReceiver
{
public:
	// event shall be deleted by caller after the call.
	virtual void onMapEditEvent(const MapEditEvent &event) = 0;
};

class Map /*: public NodeContainer*/
{
public:

	Map(IGameDef *gamedef);
	virtual ~Map();
	DISABLE_CLASS_COPY(Map);

	/*
		Drop (client) or delete (server) the map.
	*/
	virtual void drop()
	{
		delete this;
	}

	void addEventReceiver(MapEventReceiver *event_receiver);
	void removeEventReceiver(MapEventReceiver *event_receiver);
	// event shall be deleted by caller after the call.
	void dispatchEvent(const MapEditEvent &event);

	// On failure returns NULL
	MapSector * getSectorNoGenerateNoLock(v2size p2d);
	// Same as the above (there exists no lock anymore)
	MapSector * getSectorNoGenerate(v2size p2d);

	/*
		This is overloaded by ClientMap and ServerMap to allow
		their differing fetch methods.
	*/
	virtual MapSector * emergeSector(v2size p){ return NULL; }

	// Returns InvalidPositionException if not found
	MapBlock * getBlockNoCreate(v3size p);
	// Returns NULL if not found
	MapBlock * getBlockNoCreateNoEx(v3size p);

	/* Server overrides */
	virtual MapBlock * emergeBlock(v3size p, bool create_blank=true)
	{ return getBlockNoCreateNoEx(p); }

	inline const NodeDefManager * getNodeDefManager() { return m_nodedef; }

	bool isValidPosition(v3size p);

	// throws InvalidPositionException if not found
	void setNode(v3size p, MapNode n);

	// Returns a CONTENT_IGNORE node if not found
	// If is_valid_position is not NULL then this will be set to true if the
	// position is valid, otherwise false
	MapNode getNode(v3size p, bool *is_valid_position = NULL);

	/*
		These handle lighting but not faces.
	*/
	virtual void addNodeAndUpdate(v3size p, MapNode n,
			std::map<v3size, MapBlock*> &modified_blocks,
			bool remove_metadata = true);
	void removeNodeAndUpdate(v3size p,
			std::map<v3size, MapBlock*> &modified_blocks);

	/*
		Wrappers for the latter ones.
		These emit events.
		Return true if succeeded, false if not.
	*/
	bool addNodeWithEvent(v3size p, MapNode n, bool remove_metadata = true);
	bool removeNodeWithEvent(v3size p);

	// Call these before and after saving of many blocks
	virtual void beginSave() {}
	virtual void endSave() {}

	virtual void save(ModifiedState save_level) { FATAL_ERROR("FIXME"); }

	/*
		Return true unless the map definitely cannot save blocks.
	*/
	virtual bool maySaveBlocks() { return true; }

	// Server implements these.
	// Client leaves them as no-op.
	virtual bool saveBlock(MapBlock *block) { return false; }
	virtual bool deleteBlock(v3size blockpos) { return false; }

	/*
		Updates usage timers and unloads unused blocks and sectors.
		Saves modified blocks before unloading if possible.
	*/
	void timerUpdate(float dtime, float unload_timeout, s32 max_loaded_blocks,
			std::vector<v3size> *unloaded_blocks=NULL);

	/*
		Unloads all blocks with a zero refCount().
		Saves modified blocks before unloading if possible.
	*/
	void unloadUnreferencedBlocks(std::vector<v3size> *unloaded_blocks=NULL);

	// Deletes sectors and their blocks from memory
	// Takes cache into account
	// If deleted sector is in sector cache, clears cache
	void deleteSectors(std::vector<v2size> &list);

	// For debug printing. Prints "Map: ", "ServerMap: " or "ClientMap: "
	virtual void PrintInfo(std::ostream &out);

	/*
		Node metadata
		These are basically coordinate wrappers to MapBlock
	*/

	std::vector<v3size> findNodesWithMetadata(v3size p1, v3size p2);
	NodeMetadata *getNodeMetadata(v3size p);

	/**
	 * Sets metadata for a node.
	 * This method sets the metadata for a given node.
	 * On success, it returns @c true and the object pointed to
	 * by @p meta is then managed by the system and should
	 * not be deleted by the caller.
	 *
	 * In case of failure, the method returns @c false and the
	 * caller is still responsible for deleting the object!
	 *
	 * @param p node coordinates
	 * @param meta pointer to @c NodeMetadata object
	 * @return @c true on success, false on failure
	 */
	bool setNodeMetadata(v3size p, NodeMetadata *meta);
	void removeNodeMetadata(v3size p);

	/*
		Node Timers
		These are basically coordinate wrappers to MapBlock
	*/

	NodeTimer getNodeTimer(v3size p);
	void setNodeTimer(const NodeTimer &t);
	void removeNodeTimer(v3size p);

	/*
		Utilities
	*/

	// Iterates through all nodes in the area in an unspecified order.
	// The given callback takes the position as its first argument and the node
	// as its second. If it returns false, forEachNodeInArea returns early.
	template<typename F>
	void forEachNodeInArea(v3size minp, v3size maxp, F func)
	{
		auto bpmin = getNodeBlockPos(minp);
		auto bpmax = getNodeBlockPos(maxp);
		for (auto bz = bpmin.Z; bz <= bpmax.Z; bz++)
		for (auto bx = bpmin.X; bx <= bpmax.X; bx++)
		for (auto by = bpmin.Y; by <= bpmax.Y; by++) {
			// y is iterated innermost to make use of the sector cache.
			v3size bp(bx, by, bz);
			MapBlock *block = getBlockNoCreateNoEx(bp);
			v3size basep = bp * MAP_BLOCKSIZE;
			auto minx_block = rangelim(minp.X - basep.X, 0, MAP_BLOCKSIZE - 1);
			auto miny_block = rangelim(minp.Y - basep.Y, 0, MAP_BLOCKSIZE - 1);
			auto minz_block = rangelim(minp.Z - basep.Z, 0, MAP_BLOCKSIZE - 1);
			auto maxx_block = rangelim(maxp.X - basep.X, 0, MAP_BLOCKSIZE - 1);
			auto maxy_block = rangelim(maxp.Y - basep.Y, 0, MAP_BLOCKSIZE - 1);
			auto maxz_block = rangelim(maxp.Z - basep.Z, 0, MAP_BLOCKSIZE - 1);
			for (auto z_block = minz_block; z_block <= maxz_block; z_block++)
			for (auto y_block = miny_block; y_block <= maxy_block; y_block++)
			for (auto x_block = minx_block; x_block <= maxx_block; x_block++) {
				v3size p = basep + v3size(x_block, y_block, z_block);
				MapNode n = block ?
						block->getNodeNoCheck(x_block, y_block, z_block) :
						MapNode(CONTENT_IGNORE);
				if (!func(p, n))
					return;
			}
		}
	}

	bool isBlockOccluded(MapBlock *block, v3size cam_pos_nodes);
protected:
	IGameDef *m_gamedef;

	std::set<MapEventReceiver*> m_event_receivers;

	std::unordered_map<v2size, MapSector*> m_sectors;

	// Be sure to set this to NULL when the cached sector is deleted
	MapSector *m_sector_cache = nullptr;
	v2size m_sector_cache_p;

	// This stores the properties of the nodes on the map.
	const NodeDefManager *m_nodedef;

	// Can be implemented by child class
	virtual void reportMetrics(u64 save_time_us, u32 saved_blocks, u32 all_blocks) {}

	bool determineAdditionalOcclusionCheck(const v3size &pos_camera,
		const core::aabbox3d<s16> &block_bounds, v3size &check);
	bool isOccluded(const v3size &pos_camera, const v3size &pos_target,
		float step, float stepfac, float start_offset, float end_offset,
		u32 needed_count);
};

/*
	ServerMap

	This is the only map class that is able to generate map.
*/

class ServerMap : public Map
{
public:
	/*
		savedir: directory to which map data should be saved
	*/
	ServerMap(const std::string &savedir, IGameDef *gamedef, EmergeManager *emerge, MetricsBackend *mb);
	~ServerMap();

	/*
		Get a sector from somewhere.
		- Check memory
		- Check disk (doesn't load blocks)
		- Create blank one
	*/
	MapSector *createSector(v2size p);

	/*
		Blocks are generated by using these and makeBlock().
	*/
	bool blockpos_over_mapgen_limit(v3size p);
	bool initBlockMake(v3size blockpos, BlockMakeData *data);
	void finishBlockMake(BlockMakeData *data,
		std::map<v3size, MapBlock*> *changed_blocks);

	/*
		Get a block from somewhere.
		- Memory
		- Create blank
	*/
	MapBlock *createBlock(v3size p);

	/*
		Forcefully get a block from somewhere.
		- Memory
		- Load from disk
		- Create blank filled with CONTENT_IGNORE

	*/
	MapBlock *emergeBlock(v3size p, bool create_blank=true) override;

	/*
		Try to get a block.
		If it does not exist in memory, add it to the emerge queue.
		- Memory
		- Emerge Queue (deferred disk or generate)
	*/
	MapBlock *getBlockOrEmerge(v3size p3d);

	bool isBlockInQueue(v3size pos);

	void addNodeAndUpdate(v3size p, MapNode n,
			std::map<v3size, MapBlock*> &modified_blocks,
			bool remove_metadata) override;

	/*
		Database functions
	*/
	static MapDatabase *createDatabase(const std::string &name, const std::string &savedir, Settings &conf);

	// Call these before and after saving of blocks
	void beginSave() override;
	void endSave() override;

	void save(ModifiedState save_level) override;
	void listAllLoadableBlocks(std::vector<v3size> &dst);
	void listAllLoadedBlocks(std::vector<v3size> &dst);

	MapgenParams *getMapgenParams();

	bool saveBlock(MapBlock *block) override;
	static bool saveBlock(MapBlock *block, MapDatabase *db, int compression_level = -1);
	MapBlock* loadBlock(v3size p);
	// Database version
	void loadBlock(std::string *blob, v3size p3d, MapSector *sector, bool save_after_load=false);

	bool deleteBlock(v3size blockpos) override;

	void updateVManip(v3size pos);

	// For debug printing
	void PrintInfo(std::ostream &out) override;

	bool isSavingEnabled(){ return m_map_saving_enabled; }

	u64 getSeed();

	/*!
	 * Fixes lighting in one map block.
	 * May modify other blocks as well, as light can spread
	 * out of the specified block.
	 * Returns false if the block is not generated (so nothing
	 * changed), true otherwise.
	 */
	bool repairBlockLight(v3size blockpos,
		std::map<v3size, MapBlock *> *modified_blocks);

	void transformLiquids(std::map<v3size, MapBlock*> & modified_blocks,
			ServerEnvironment *env);

	void transforming_liquid_add(v3size p);

	MapSettingsManager settings_mgr;

protected:

	void reportMetrics(u64 save_time_us, u32 saved_blocks, u32 all_blocks) override;

private:
	friend class LuaVoxelManip;

	// Emerge manager
	EmergeManager *m_emerge;

	std::string m_savedir;
	bool m_map_saving_enabled;

	int m_map_compression_level;

	std::set<v3size> m_chunks_in_progress;

	// Queued transforming water nodes
	UniqueQueue<v3size> m_transforming_liquid;
	f32 m_transforming_liquid_loop_count_multiplier = 1.0f;
	u32 m_unprocessed_count = 0;
	u64 m_inc_trending_up_start_time = 0; // milliseconds
	bool m_queue_size_timer_started = false;

	/*
		Metadata is re-written on disk only if this is true.
		This is reset to false when written on disk.
	*/
	bool m_map_metadata_changed = true;
	MapDatabase *dbase = nullptr;
	MapDatabase *dbase_ro = nullptr;

	// Map metrics
	MetricGaugePtr m_loaded_blocks_gauge;
	MetricCounterPtr m_save_time_counter;
	MetricCounterPtr m_save_count_counter;
};


#define VMANIP_BLOCK_DATA_INEXIST     1
#define VMANIP_BLOCK_CONTAINS_CIGNORE 2

class MMVManip : public VoxelManipulator
{
public:
	MMVManip(Map *map);
	virtual ~MMVManip() = default;

	virtual void clear()
	{
		VoxelManipulator::clear();
		m_loaded_blocks.clear();
	}

	void initialEmerge(v3size blockpos_min, v3size blockpos_max,
		bool load_if_inexistent = true);

	// This is much faster with big chunks of generated data
	void blitBackAll(std::map<v3size, MapBlock*> * modified_blocks,
		bool overwrite_generated = true);

	/*
		Creates a copy of this VManip including contents, the copy will not be
		associated with a Map.
	*/
	MMVManip *clone() const;

	// Reassociates a copied VManip to a map
	void reparent(Map *map);

	// Is it impossible to call initialEmerge / blitBackAll?
	inline bool isOrphan() const { return !m_map; }

	bool m_is_dirty = false;

protected:
	MMVManip() {};

	// may be null
	Map *m_map = nullptr;
	/*
		key = blockpos
		value = flags describing the block
	*/
	std::map<v3size, u8> m_loaded_blocks;
};
