
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
#include "actionspace/ActionspaceManager.hpp"
#include "actionspace/ActionspaceRoomNode.hpp"


act::room::ActionspaceManager::ActionspaceManager()
	: RoomNodeManagerBase("actionspaceManager")
{
	
}

act::room::ActionspaceManager::~ActionspaceManager()
{
}

void act::room::ActionspaceManager::setup()
{
	
}

void act::room::ActionspaceManager::update()
{

}

act::room::RoomNodeBaseRef act::room::ActionspaceManager::drawMenu()
{
	act::room::RoomNodeBaseRef node = nullptr;

	if (ImGui::Button("add Device")) {
		node = addActionspace("actionspace", glm::vec3(0.0f, 0.1f, 0.0f));
	}

	return node;
}
	
ci::Json act::room::ActionspaceManager::toJson()
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

void act::room::ActionspaceManager::fromJson(ci::Json json)
{
	if (json.contains("nodes")) {
		auto nodesJson = json["nodes"];
		for (auto&& node : nodesJson) {
			
			std::string name = "";
			util::setValueFromJson(node, "name", name);

			glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f);
			util::setValueFromJson(node, "position", position);

			auto actionspaceNode = addActionspace(name, position);

			actionspaceNode->fromJson(node);

			refreshLists();
		}
	}
}

void act::room::ActionspaceManager::refreshLists()
{

}

act::room::RoomNodeBaseRef act::room::ActionspaceManager::addActionspace(std::string name, glm::vec3 position)
{
	auto node = act::room::ActionspaceRoomNode::create(name, position);
	m_nodes.push_back(node);
	return node;
}

