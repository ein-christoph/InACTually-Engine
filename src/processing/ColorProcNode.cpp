
/*
	InACTually
	> interactive theater for actual acts
	> this file is part of the "InACTually Engine", a MediaServer for driving all technology

	Copyright (c) 2021–2025 Lars Engeln, Fabian Töpfer
	Copyright (c) 2025 InACTually Community
	Licensed under the MIT License.
	See LICENSE file in the project root for full license information.

	This file is created and substantially modified: 2021

	contributors:
	Lars Engeln - mail@lars-engeln.de
*/

#include "procpch.hpp"
#include "ColorProcNode.hpp"


act::proc::ColorProcNode::ColorProcNode() : ProcNodeBase("Color") {
	m_drawSize = glm::ivec2(200, 200);

	init = true;

	m_colorPort = createColorOutput("color");

	auto triggerInput = createBoolInput("trigger", [&](bool triggered) {
		if (triggered)
			m_colorPort->send(m_color);
		});
}

act::proc::ColorProcNode::~ColorProcNode() {
}

void act::proc::ColorProcNode::update() {
	if (init) {
		m_colorPort->send(m_color);
		init = false;
	}
}

void act::proc::ColorProcNode::draw() {
	beginNodeDraw();

	ImGui::SetColorEditOptions(ImGuiColorEditFlags_DisplayHSV);
	if (ImGui::ColorPicker3("Color", m_color.ptr())) {
		preventDrag(true);
		m_colorPort->send(m_color);
	}
	else {
		preventDrag(false);
	}

	endNodeDraw();
}

ci::Json act::proc::ColorProcNode::toParams() {
	ci::Json json = ci::Json::object();
	json["r"] = m_color.r;
	json["g"] = m_color.g;
	json["b"] = m_color.b;
	return json;
}

void act::proc::ColorProcNode::fromParams(ci::Json json) {
	util::setValueFromJson(json, "r", m_color.r);
	util::setValueFromJson(json, "g", m_color.g);
	util::setValueFromJson(json, "b", m_color.b);

	m_colorPort->send(m_color);
}
