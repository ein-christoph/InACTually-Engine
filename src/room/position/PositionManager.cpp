
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
#include "position/PositionManager.hpp"

#include "position/PositionRoomNode.hpp"


act::room::PositionManager::PositionManager()
	: RoomNodeManagerBase("positionManager")
{
	refreshLists();
}

act::room::PositionManager::~PositionManager()
{
}

void act::room::PositionManager::setup()
{
}

act::room::RoomNodeBaseRef act::room::PositionManager::drawMenu()
{
	act::room::RoomNodeBaseRef node;

	if (ImGui::Button("add Position")) {
		node = addPosition(glm::vec3(0.0f, 0.1f, 0.0f));
	}

	return node;
}

void act::room::PositionManager::update()
{

	for (auto&& node : m_nodes) {
		node->update();
	}

}

ci::Json act::room::PositionManager::toJson()
{
	auto json = ci::Json::object();

	ci::Json nodes = ci::Json::array();
	for (auto&& node : m_nodes) {
		auto nodeJson = node->toJson();

		nodes.push_back(nodeJson);
	}
	json["nodes"] = nodes;

	return json;
	
}

void act::room::PositionManager::fromJson(ci::Json json)
{
	if (json.contains("nodes")) {
		auto nodesJson = json["nodes"];
		for (auto&& nodeJson : nodesJson) {

			//util::setValueFromJson(nodeJson, "name", name);
			auto node = addPosition(glm::vec3(0.0f));
			node->fromJson(nodeJson);

			refreshLists();
		}
	}
}


void act::room::PositionManager::refreshLists()
{

}


act::room::RoomNodeBaseRef act::room::PositionManager::addPosition(glm::vec3 position)
{
	auto node = PositionRoomNode::create("position");
	node->setPosition(position);
	m_nodes.push_back(node);
	refreshLists();
	return node;

	return nullptr;
}