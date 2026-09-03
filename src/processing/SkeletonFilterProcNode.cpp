
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
#include "SkeletonFilterProcNode.hpp"


act::proc::SkeletonFilterProcNode::SkeletonFilterProcNode() : ProcNodeBase("SkeletonFilter") {
	m_drawSize = glm::ivec2(250, 250);

	m_positionOutPort = createVec3Output("Position");

	auto skeleton = InputPort<std::tuple<uint32_t, k4abt_skeleton_t>>::create(PT_BODY, "Skeleton", [&](std::tuple<uint32_t, k4abt_skeleton_t> skeletonTuple) { this->onSkeleton(skeletonTuple); });
	m_inputPorts.push_back(skeleton);

	m_currentJoint = K4ABT_JOINT_SPINE_CHEST;
}

act::proc::SkeletonFilterProcNode::~SkeletonFilterProcNode() {
}


void act::proc::SkeletonFilterProcNode::setup() {
}

void act::proc::SkeletonFilterProcNode::update() {
}

void act::proc::SkeletonFilterProcNode::draw() {
	beginNodeDraw();

	ImGui::SetNextItemWidth(m_drawSize.x - ImGui::CalcTextSize("Skeleton").x);

	ImGui::Combo("Joint Selection", &m_currentJoint, m_jointSelection, IM_ARRAYSIZE(m_jointSelection));


	if (m_currentJointPosition.x > -1000000.0f)	{
		ImGui::Text("Current Joint [xyz]: [ %3.2lf,  %3.2lf, %3.2lf ]", m_currentJointPosition.x, m_currentJointPosition.y, m_currentJointPosition.z);
	}

	endNodeDraw();
}

void act::proc::SkeletonFilterProcNode::onSkeleton(std::tuple<uint32_t, k4abt_skeleton_t> event) {
	uint32_t bodyID = std::get<0>(event);
	k4abt_skeleton_t skeleton = std::get<1>(event);
	k4a_float3_t::_xyz skelPos = skeleton.joints[m_currentJoint].position.xyz;

	m_currentJointPosition = glm::vec3(skelPos.x, skelPos.y, skelPos.z);

	m_positionOutPort->send(m_currentJointPosition);
}


ci::Json act::proc::SkeletonFilterProcNode::toParams() {
	ci::Json json = ci::Json::object();
	json["selectedJoint"] = m_currentJoint;
	return json;
}

void act::proc::SkeletonFilterProcNode::fromParams(ci::Json json) {
	util::setValueFromJson(json, "selectedJoint", m_currentJoint);
}