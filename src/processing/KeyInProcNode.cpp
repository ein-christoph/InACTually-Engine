
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
#include "KeyInProcNode.hpp"


act::proc::KeyInProcNode::KeyInProcNode() : ProcNodeBase("KeyIn") {
	m_drawSize = glm::ivec2(200, 200);

	m_key = ImGuiKey_Space;
	init = true;

	m_pressedPort = createBoolOutput("is pressed");
}

act::proc::KeyInProcNode::~KeyInProcNode() {
}

void act::proc::KeyInProcNode::update() {
	if (init) {
		//m_pressedPort->send(true);
		init = false;
	}

	if (m_isEnteringKey) {
		m_key = getCurrentPressedKey();
		if (m_key != ImGuiKey_None) {
			m_isEnteringKey = false;
		}
	}
}

void act::proc::KeyInProcNode::draw() {
	beginNodeDraw();

	if (!ImGui::IsAnyItemFocused() && ImGui::IsKeyDown(m_key)) {
		m_pressedPort->send(true);
	}
	else if (ImGui::IsKeyReleased(m_key)) {
		m_pressedPort->send(false);
	}

	if (!m_isEnteringKey) {
		if (ImGui::Button(ImGui::GetKeyName(m_key))) {
			m_isEnteringKey = true;
		}
	}
	else {
		ImGui::Text("Enter a new key.");
	}

	endNodeDraw();
}

ci::Json act::proc::KeyInProcNode::toParams() {
	ci::Json json = ci::Json::object();
	json["key"] = (int)m_key;

	return json;
}

void act::proc::KeyInProcNode::fromParams(ci::Json json) {
	int keyEnum;
	util::setValueFromJson(json, "key", keyEnum);
	m_key = (ImGuiKey)keyEnum;
}

ImGuiKey act::proc::KeyInProcNode::getCurrentPressedKey()
{
	if (ImGui::IsKeyDown(ImGuiKey_Space))
		return ImGuiKey_Space;

	ImGuiKey start_key = ImGuiKey_LeftArrow;
	for (ImGuiKey key = start_key; key < ImGuiKey_F1; key = (ImGuiKey)(key + 1)) { 
		if (ImGui::IsKeyDown(key)) 
			return key; 
	}

	return ImGuiKey_None;
}
