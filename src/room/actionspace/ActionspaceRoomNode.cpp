
/*
	InACTually
	> interactive theater for actual acts
	> this file is part of the "InACTually Engine", a MediaServer for driving all technology

	Copyright (c) 2021–2025 Lars Engeln, Fabian Töpfer
	Copyright (c) 2025 InACTually Community
	Licensed under the MIT License.
	See LICENSE file in the project root for full license information.

	This file is created and substantially modified: 2024

	contributors:
	Lars Engeln - mail@lars-engeln.de
*/

#include "roompch.hpp"
#include "actionspace/ActionspaceRoomNode.hpp"


act::room::ActionspaceRoomNode::ActionspaceRoomNode(std::string name, glm::vec3 position, glm::vec3 rotation, float radius, act::UID replyUID)
	: RoomNodeBase(name, position, rotation, radius, replyUID)
{
	m_bounding = BoundingSphere::create(position, radius);
}

act::room::ActionspaceRoomNode::~ActionspaceRoomNode()
{
}

void act::room::ActionspaceRoomNode::setup()
{
}

void act::room::ActionspaceRoomNode::update()
{
}

void act::room::ActionspaceRoomNode::draw()
{
	m_bounding->draw();
}

void act::room::ActionspaceRoomNode::drawSpecificSettings()
{
}

ci::Json act::room::ActionspaceRoomNode::toParams()
{
	ci::Json params = ci::Json();
	return params;
}

void act::room::ActionspaceRoomNode::fromParams(ci::Json params)
{

	int type = (int)m_type;
	util::setValueFromJson(params, "type", type);
	m_type = (ASType)type;
	
	if (params.contains("typename"))
		m_type = fromTypeString(params["typename"]);

	if (params.contains("size")) {
		int x, y, z = -1;
		ci::Json size = params["size"];
		util::setValueFromJson(size, "x", x);
		util::setValueFromJson(size, "y", y);
		util::setValueFromJson(size, "z", z);
	}
}

act::room::ASType act::room::ActionspaceRoomNode::fromTypeString(std::string typestring)
{
	if (typestring == "sphere")
		return AST_SPHERE;
	if (typestring == "cylinder")
		return AST_CYLINDER;
	if (typestring == "cuboid")
		return AST_CUBOID;
	if (typestring == "cone")
		return AST_CONE;
	if (typestring == "free")
		return AST_FREE;

	return AST_UNKOWN;
}
