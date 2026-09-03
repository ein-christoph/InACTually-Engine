
/*
	InACTually
	> interactive theater for actual acts
	> this file is part of the "InACTually Engine", a MediaServer for driving all technology

	Copyright (c) 2021–2025 Lars Engeln, Fabian Töpfer
	Copyright (c) 2025 InACTually Community
	Licensed under the MIT License.
	See LICENSE file in the project root for full license information.

	This file is created and substantially modified: 2022

	contributors:
	Lars Engeln - mail@lars-engeln.de
*/

#include "procpch.hpp"
#include "SkeletonMovementProcNode.hpp"


act::proc::SkeletonMovementProcNode::SkeletonMovementProcNode() : ProcNodeBase("SkeletonMovement") {

	m_drawSize = glm::ivec2(250, 250);

	auto body = InputPort<room::BodyRef>::create(PT_BODY, "Body", [&](room::BodyRef body) { this->onBody(body); });
	m_inputPorts.push_back(body);

	m_localMovementPort = createNumberOutput("localMovement");
	m_globalMovementPort = createNumberOutput("globalMovement");

	m_localMovement = 0.0f;
	m_globalMovement = 0.0f;
	
	m_oldBody = nullptr;
	m_scaleValue = 1.0f;
}

act::proc::SkeletonMovementProcNode::~SkeletonMovementProcNode() {
}

void act::proc::SkeletonMovementProcNode::onBody(room::BodyRef body)
{
	if (m_oldBody == nullptr) {
		m_oldBody = body;
		return;
	}

	m_localMovement = calcLocalMovement(body) * m_scaleValue;
	m_globalMovement = calcGlobalMovement(body) * m_scaleValue;

	m_localMovementPort->send(m_localMovement);
	m_globalMovementPort->send(m_globalMovement);


	m_oldBody = body;
}


float act::proc::SkeletonMovementProcNode::calcLocalMovement(room::BodyRef body) {
	if (m_oldBody == nullptr) {
		return 0;
	}

	int numJoints = body->joints.size();

	float totalDist = 0.0f;
	float distX, distY, distZ;

	for (int i = 0; i < numJoints; i++) {


		glm::vec3 currJointPos = body->joints[i]->position;
		glm::vec3 oldJointPos = m_oldBody->joints[i]->position;

		distX = abs(currJointPos.x - oldJointPos.x);
		distY = abs(currJointPos.y - oldJointPos.y);
		distZ = abs(currJointPos.z - oldJointPos.z);

		totalDist += distX + distY + distZ;
	}

	if (totalDist == 0) {
		return m_localMovement / m_scaleValue;
	}

	return totalDist / (float)numJoints;
}

float act::proc::SkeletonMovementProcNode::calcGlobalMovement(room::BodyRef body) {
	if (m_oldBody == nullptr) {
		return 0;
	}

	float totalDist = 0.0f;
	float distX, distY, distZ;

	glm::vec3 currJointPos = body->joints[room::BJT_SPINE_CHEST]->position;
	glm::vec3 oldJointPos = m_oldBody->joints[room::BJT_SPINE_CHEST]->position;

	distX = abs(currJointPos.x - oldJointPos.x);
	distY = abs(currJointPos.y - oldJointPos.y);
	distZ = abs(currJointPos.z - oldJointPos.z);


	if (distX + distY + distZ == 0) {
		return m_globalMovement / m_scaleValue;
	}


	return distX + distY + distZ;
}

void act::proc::SkeletonMovementProcNode::update() {
}

void act::proc::SkeletonMovementProcNode::draw() {
	beginNodeDraw();

	ImGui::SetNextItemWidth(m_drawSize.x - 50);

	ImGui::Value("local movement: ", m_localMovement, "%.3f");
	ImGui::Value("global movement: ", m_globalMovement, "%.3f");

	ImGui::SetNextItemWidth(m_drawSize.x * 2);
	preventDrag(ImGui::SliderFloat("scale value", &m_scaleValue, 0.1f, 5.0f));

	endNodeDraw();
}

ci::Json act::proc::SkeletonMovementProcNode::toParams() {
	ci::Json json = ci::Json::object();
 
	return json; 
}

void act::proc::SkeletonMovementProcNode::fromParams(ci::Json json) {
}
