
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
#include "HandProcNode.hpp"


act::proc::HandProcNode::HandProcNode() : ProcNodeBase("Hand") {
	m_drawSize = glm::ivec2(250, 250);

	m_leftHandClosed = createBoolOutput("is left closed");
	m_leftHandOpend = createBoolOutput("is left opened");
	m_rightHandClosed = createBoolOutput("is right closed");
	m_rightHandOpend = createBoolOutput("is right opened");
	//m_orientation = OutputPort<glm::quat>::create(PT_QUAT, "orientation");

	auto skeleton = InputPort<std::tuple<uint32_t, k4abt_skeleton_t>>::create(PT_BODY, "Skeleton", [&](std::tuple<uint32_t, k4abt_skeleton_t> skeletonTuple) { onBody(std::get<1>(skeletonTuple)); });
	m_inputPorts.push_back(skeleton);
}

act::proc::HandProcNode::~HandProcNode() {
}

void act::proc::HandProcNode::setup() {
}

void act::proc::HandProcNode::update() {
}

void act::proc::HandProcNode::draw() {
	beginNodeDraw();

	{
		std::stringstream txt;
		txt << "left:  " << m_distanceLeft;
		ImGui::Text(txt.str().c_str());
	}
	{
		std::stringstream txt;
		txt << "right: " << m_distanceRight;
		ImGui::Text(txt.str().c_str());
	}

	endNodeDraw();
}

void act::proc::HandProcNode::onBody(k4abt_skeleton_t body)
{
	auto leftTip	= body.joints[K4ABT_JOINT_HANDTIP_LEFT].position.xyz;
	auto rightTip	= body.joints[K4ABT_JOINT_HANDTIP_RIGHT].position.xyz;
	auto leftWrist	= body.joints[K4ABT_JOINT_WRIST_LEFT].position.xyz;
	auto rightWrist = body.joints[K4ABT_JOINT_WRIST_RIGHT].position.xyz;
	auto leftHand	= body.joints[room::BJT_HAND_LEFT].position.xyz;

	//m_distanceLeft	= ci::dot(glm::vec3(leftTip.x, leftTip.y, leftTip.z) - glm::vec3(leftHand.x, leftHand.y, leftHand.z), glm::vec3(leftWrist.x, leftWrist.y, leftWrist.z) - glm::vec3(leftHand.x, leftHand.y, leftHand.z));
	m_distanceRight = ci::distance(glm::vec3(rightTip.x, rightTip.y, rightTip.z), glm::vec3(rightWrist.x, rightWrist.y, rightWrist.z));
	m_distanceLeft = ci::distance(glm::vec3(leftTip.x, leftTip.y, leftTip.z), glm::vec3(leftWrist.x, leftWrist.y, leftWrist.z));
	
	if (m_distanceLeft < 0.2f) m_leftHandClosed->send(true);
	else m_leftHandOpend->send(true);
	if (m_distanceRight < 0.2f) m_rightHandClosed->send(true);
	else m_rightHandOpend->send(true);
}

ci::Json act::proc::HandProcNode::toParams() {
	ci::Json json = ci::Json::object();
	return json;
}

void act::proc::HandProcNode::fromParams(ci::Json json) {
}