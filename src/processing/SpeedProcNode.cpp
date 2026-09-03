
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
#include "SpeedProcNode.hpp"

#include <numeric>


act::proc::SpeedProcNode::SpeedProcNode() : ProcNodeBase("Speed") {
	m_drawSize = glm::ivec2(300, 300);

	auto positionInput = createVec3Input("position", [&](glm::vec3 pos) { this->onPosition(pos); });
	
	m_speedPort = createNumberOutput("speed");
	

}

act::proc::SpeedProcNode::~SpeedProcNode() {
}

void act::proc::SpeedProcNode::update() {
}

void act::proc::SpeedProcNode::draw() {
	beginNodeDraw();
 
	bool prvntDrag = false;
	if (ImGui::SliderFloat("factor", &m_factor, 0.01f, 10.0f)) {
		prvntDrag = true;
	}

	preventDrag(prvntDrag);
	
	endNodeDraw();
}

ci::Json act::proc::SpeedProcNode::toParams() {
	ci::Json json = ci::Json::object();
	 
	return json;
}

void act::proc::SpeedProcNode::fromParams(ci::Json json) {
}

void act::proc::SpeedProcNode::onPosition(glm::vec3 position) {

	float distance = ci::distance(position, m_lastPosition);
	m_lastPosition = position;

	float speed = distance * m_factor;

	m_speedPort->send(speed);
}
