
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
#include "HeadProcNode.hpp"


act::proc::HeadProcNode::HeadProcNode() : ProcNodeBase("Head") {
	m_drawSize = glm::ivec2(250, 250);

	m_direction = glm::vec3(1.0f, 0.0f, 0.0f);
	m_horizontal = glm::vec3(0.0f, 0.0f, 1.0f);
	m_vertical = glm::vec3(0.0f, 1.0f, 0.0f);

	m_directionOutput = createVec3Output("direction");
	//m_orientation = OutputPort<glm::quat>::create(PT_QUAT, "orientation");


	auto skeleton = InputPort<std::tuple<uint32_t, k4abt_skeleton_t>>::create(PT_BODY, "Skeleton", [&](std::tuple<uint32_t, k4abt_skeleton_t> skeletonTuple) { onBody(std::get<1>(skeletonTuple)); });
	m_inputPorts.push_back(skeleton);
}

act::proc::HeadProcNode::~HeadProcNode() {
}

void act::proc::HeadProcNode::setup() {
}

void act::proc::HeadProcNode::update() {
}

void act::proc::HeadProcNode::draw() {
	beginNodeDraw();

	{
		std::stringstream txt;
		txt << "horzontal - x: " << m_horizontal.x << ", y: " << m_horizontal.y << ", z: " << m_horizontal.z;
		ImGui::Text(txt.str().c_str());
	}
	{
		std::stringstream txt;
		txt << "vertical - x: " << m_vertical.x << ", y: " << m_vertical.y << ", z: " << m_vertical.z;
		ImGui::Text(txt.str().c_str());
	}
	{
		std::stringstream txt;
		txt << "direction - x: " << m_direction.x << ", y: " << m_direction.y << ", z: " << m_direction.z;
		ImGui::Text(txt.str().c_str());
	}

	endNodeDraw();
}

void act::proc::HeadProcNode::onBody(k4abt_skeleton_t body)
{
	//auto kQ = body.joints[K4ABT_JOINT_HEAD].orientation;
	//glm::quat q = glm::quat(kQ.wxyz.w, kQ.wxyz.x, kQ.wxyz.y, kQ.wxyz.z);
	//m_orientation->send(q);
	
	//auto left	= toVec(body.joints[K4ABT_JOINT_SHOULDER_LEFT].position.xyz);
	//auto right	= toVec(body.joints[K4ABT_JOINT_SHOULDER_RIGHT].position.xyz);
	//auto below	= toVec(body.joints[K4ABT_JOINT_SPINE_CHEST].position.xyz);
	//auto above	= toVec(body.joints[K4ABT_JOINT_HEAD].position.xyz);



	auto left	= toVec(body.joints[K4ABT_JOINT_EAR_LEFT].position.xyz);
	auto right	= toVec(body.joints[K4ABT_JOINT_EAR_RIGHT].position.xyz);
	auto below	= toVec(body.joints[K4ABT_JOINT_HEAD].position.xyz);
	//auto nose	= toVec(body.joints[K4ABT_JOINT_NOSE].position.xyz);
	auto above	= right + ((left - right) * 2.0f);


	
	m_horizontal	= ci::normalize(left - right);
	m_vertical		= ci::normalize(below - above);
	m_direction		= ci::normalize(ci::cross(m_vertical, m_horizontal));


	//m_direction = ci::normalize(nose - above);

	m_directionOutput->send(m_direction);
}

ci::Json act::proc::HeadProcNode::toParams() {
	ci::Json json = ci::Json::object();

	return json;
}

void act::proc::HeadProcNode::fromParams(ci::Json json) {
}

glm::vec3 act::proc::HeadProcNode::toVec(k4a_float3_t::_xyz xyz)
{
	return glm::vec3(xyz.x, xyz.y, xyz.z);
}
