/*
Minetest
Copyright (C) 2010-2013 celeron55, Perttu Ahola <celeron55@gmail.com>
Copyright (C) 2013-2020 Minetest core developers & community

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

#include "object_properties.h"
#include "serveractiveobject.h"
#include <quaternion.h>
#include "util/numeric.h"

class UnitSAO : public ServerActiveObject
{
public:
	UnitSAO(ServerEnvironment *env, v3d pos);
	virtual ~UnitSAO() = default;

	u16 getHP() const { return m_hp; }
	// Use a function, if isDead can be defined by other conditions
	bool isDead() const { return m_hp == 0; }

	// Rotation
	void setRotation(v3d rotation) { m_rotation = rotation; }
	const v3d &getRotation() const { return m_rotation; }
	const v3d getTotalRotation() const {
		// This replicates what happens clientside serverside
		core::matrix4 rot;
		setPitchYawRoll(rot, -m_rotation);
		v3d res;
		// First rotate by m_rotation, then rotate by the automatic rotate yaw
		(core::quaternion(v3d(0, -m_rotation_add_yaw * core::DEGTORAD, 0))
				* core::quaternion(rot.getRotationDegrees() * core::DEGTORAD))
				.toEuler(res);
		return res * core::RADTODEG;
	}
	v3d getRadRotation() { return m_rotation * core::DEGTORAD; }

	// Deprecated
	f64 getRadYawDep() const { return (m_rotation.Y + 90.) * core::DEGTORAD; }

	// Armor groups
	inline bool isImmortal() const
	{
		return itemgroup_get(getArmorGroups(), "immortal");
	}
	void setArmorGroups(const ItemGroupList &armor_groups);
	const ItemGroupList &getArmorGroups() const;

	// Animation
	void setAnimation(v2f frame_range, float frame_speed, float frame_blend,
			bool frame_loop);
	void getAnimation(v2f *frame_range, float *frame_speed, float *frame_blend,
			bool *frame_loop);
	void setAnimationSpeed(float frame_speed);

	// Bone position
	void setBonePosition(const std::string &bone, v3d position, v3d rotation);
	void getBonePosition(const std::string &bone, v3d *position, v3d *rotation);

	// Attachments
	ServerActiveObject *getParent() const;
	inline bool isAttached() const { return getParent(); }
	void setAttachment(int parent_id, const std::string &bone, v3d position,
			v3d rotation, bool force_visible);
	void getAttachment(int *parent_id, std::string *bone, v3d *position,
			v3d *rotation, bool *force_visible) const;
	void clearChildAttachments();
	void clearParentAttachment();
	void addAttachmentChild(int child_id);
	void removeAttachmentChild(int child_id);
	const std::unordered_set<int> &getAttachmentChildIds() const;

	// Object properties
	ObjectProperties *accessObjectProperties();
	void notifyObjectPropertiesModified();
	void sendOutdatedData();

	// Update packets
	std::string generateUpdateAttachmentCommand() const;
	std::string generateUpdateAnimationSpeedCommand() const;
	std::string generateUpdateAnimationCommand() const;
	std::string generateUpdateArmorGroupsCommand() const;
	static std::string generateUpdatePositionCommand(const v3d &position,
			const v3d &velocity, const v3d &acceleration, const v3d &rotation,
			bool do_interpolate, bool is_movement_end, f64 update_interval);
	std::string generateSetPropertiesCommand(const ObjectProperties &prop) const;
	static std::string generateUpdateBonePositionCommand(const std::string &bone,
			const v3d &position, const v3d &rotation);
	void sendPunchCommand();

protected:
	u16 m_hp = 1;

	v3d m_rotation;
	f64 m_rotation_add_yaw = 0;

	ItemGroupList m_armor_groups;

	// Object properties
	bool m_properties_sent = true;
	ObjectProperties m_prop;

	// Stores position and rotation for each bone name
	std::unordered_map<std::string, core::vector2d<v3d>> m_bone_position;

	int m_attachment_parent_id = 0;

private:
	void onAttach(int parent_id);
	void onDetach(int parent_id);

	std::string generatePunchCommand(u16 result_hp) const;

	// Armor groups
	bool m_armor_groups_sent = false;

	// Animation
	v2f m_animation_range;
	float m_animation_speed = 0.0f;
	float m_animation_blend = 0.0f;
	bool m_animation_loop = true;
	bool m_animation_sent = false;
	bool m_animation_speed_sent = false;

	// Bone positions
	bool m_bone_position_sent = false;

	// Attachments
	std::unordered_set<int> m_attachment_child_ids;
	std::string m_attachment_bone = "";
	v3d m_attachment_position;
	v3d m_attachment_rotation;
	bool m_attachment_sent = false;
	bool m_force_visible = false;
};
