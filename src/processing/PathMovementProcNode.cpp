
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

#include "procpch.hpp"
#include "PathMovementProcNode.hpp"

act::proc::PathMovementProcNode::PathMovementProcNode() : ProcNodeBase("PathMovement") {
	m_drawSize = glm::ivec2(200, 200);

	m_position = glm::vec3(0.0f, 0.0f, 0.0f);

	auto tIn = createNumberInput("position on path", [&](float t) { evaluate(t); });

	m_positionPort = createVec3Output("position");
}

act::proc::PathMovementProcNode::~PathMovementProcNode() {
}

void act::proc::PathMovementProcNode::update() {
}

void act::proc::PathMovementProcNode::draw() {
	beginNodeDraw();

	ImGui::SetNextItemWidth(m_drawSize.x);

	if (ImGui::DragFloat3("center", &m_position, 0.01f)) {
		preventDrag(true);
		update();
	}
	else {
		preventDrag(false);
	}

	endNodeDraw();
}

ci::Json act::proc::PathMovementProcNode::toParams() {
	ci::Json json = ci::Json::object();
	json["x"]		= m_position.x;
	json["y"]		= m_position.y;
	json["z"]		= m_position.z;
	
	return json;
}

void act::proc::PathMovementProcNode::fromParams(ci::Json json) {
	util::setValueFromJson(json, "x", m_position.x);
	util::setValueFromJson(json, "y", m_position.y);
	util::setValueFromJson(json, "z", m_position.z);
}

void act::proc::PathMovementProcNode::evaluate(float t)
{

}
