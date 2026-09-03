
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
	Fabian Töpfer
	Lars Engeln - mail@lars-engeln.de
*/

#include "procpch.hpp"
#include "LinkerProcNode.hpp"


act::proc::LinkerProcNode::LinkerProcNode()
	: ProcNodeBase("Linker", act::proc::NT_INPUT) 
{
	m_isInput = true;
	isInputChanged();
};

act::proc::LinkerProcNode::~LinkerProcNode() {};

void act::proc::LinkerProcNode::draw() {

	beginNodeDraw(NDS_ACTIVE);

	if (m_isInput) {
		if (ImGui::Button("Switch to Output")) { m_isInput = false; isInputChanged(); }
	}
	else {
		if (ImGui::Button("Switch to Input")) { m_isInput = true; isInputChanged(); }
	}

	ImGui::NewLine();

	ImGui::BeginGroup();
	ImGui::Text("Port Name: ");

	ImGui::SetNextItemWidth(200);
	if (ImGui::InputText("", &m_title)) { // change to setTitle() ?
		if (m_externalInputPort != nullptr) {
			m_externalInputPort->setCaption(getTitle());
			m_internalOutputPort->setCaption(getTitle());
		}
		else if (m_externalOutputPort != nullptr) {
			m_externalOutputPort->setCaption(getTitle());
			m_internalInputPort->setCaption(getTitle());
		}
	}

	ImGui::EndGroup();

	endNodeDraw();
};

void act::proc::LinkerProcNode::drawPorts(int width)
{
	if (m_isInput)
		m_internalOutputPort->draw(width);
	else
		m_internalInputPort->draw(width);
};

void act::proc::LinkerProcNode::isInputChanged() {
	if (m_isInput) {
		m_nodeType = act::proc::NT_INPUT;

		m_inputPorts.resize(0);
		m_outputPorts.resize(0);

		m_internalInputPort = nullptr;
		m_externalOutputPort = nullptr;

		m_internalOutputPort = OutputPort<float>::create(PT_GENERIC, "Generic Output");
		m_externalInputPort = InputPort<float>::create(PT_GENERIC, "Generic Input", [&](float d) {});

		m_outputPorts.push_back(m_internalOutputPort);
		m_inputPorts.push_back(m_externalInputPort);
	}
	else {
		m_nodeType = act::proc::NT_OUTPUT;

		m_inputPorts.resize(0);
		m_outputPorts.resize(0);

		m_internalInputPort = InputPort<float>::create(PT_GENERIC, "Generic Input", [&](float d) {});
		m_externalOutputPort = OutputPort<float>::create(PT_GENERIC, "Generic Output");

		m_internalOutputPort = nullptr;
		m_externalInputPort = nullptr;

		m_inputPorts.push_back(m_internalInputPort);
		m_outputPorts.push_back(m_externalOutputPort);
	}
};

ci::Json act::proc::LinkerProcNode::toParams() {
	ci::Json json = ci::Json::object();
	if (m_isInput)
		json["isInput"] = 1;
	else
		json["isInput"] = 0;

	return json;
};

void act::proc::LinkerProcNode::fromParams(ci::Json json) {
	if (json["isInput"] == 0) {
		m_isInput = false;
	}
	else {
		m_isInput = true;
	}

	isInputChanged();
};

act::proc::PortBaseRef act::proc::LinkerProcNode::getOutputPortByName(std::string name) {
	if (m_isInput)
		return m_internalOutputPort;
	return m_externalOutputPort;
};

act::proc::PortBaseRef act::proc::LinkerProcNode::getInputPortByName(std::string name) {
	if (m_isInput)
		return m_externalInputPort;
	return m_internalInputPort;
};


void act::proc::LinkerProcNode::update() {

};

act::proc::PortBaseRef act::proc::LinkerProcNode::connectToPort(PortBaseRef toPort) {
	if (m_isInput) {
		setInternalOutputPort(toPort);
		return m_internalOutputPort;
	}
	else {
		setInternalInputPort(toPort);
		return m_externalOutputPort;
	}
};

act::proc::PortBaseRef act::proc::LinkerProcNode::connectFromPort(PortBaseRef fromPort) {
	if (m_isInput) {
		setInternalOutputPort(fromPort);
		return m_externalInputPort;
	}
	else {
		setInternalInputPort(fromPort);
		return m_internalInputPort;
	}
};

#define SET_LINKERPORTS_OUTPUT(port, type) case port:{  \
		auto internalOutputPort = OutputPort<type>::create(port, toPort->getName());\
		m_internalOutputPort = internalOutputPort;\
		m_externalInputPort = InputPort<type>::create(port, toPort->getName(), [internalOutputPort](type d) { internalOutputPort->send(d); });\
		break;\
	}

act::proc::PortBaseRef act::proc::LinkerProcNode::setInternalOutputPort(PortBaseRef toPort) {

	//only init new ports if the corresponding counter port is still not connected
	if (m_externalInputPort->getType() != PT_GENERIC)
		return m_internalOutputPort;

	switch (toPort->getType()) {

		SET_LINKERPORTS_OUTPUT(PT_IMAGE, cv::UMat)
			SET_LINKERPORTS_OUTPUT(PT_NUMBER, float)
			SET_LINKERPORTS_OUTPUT(PT_TEXT, std::string)
			SET_LINKERPORTS_OUTPUT(PT_BOOL, bool)
			SET_LINKERPORTS_OUTPUT(PT_COLOR, ci::Color)
			SET_LINKERPORTS_OUTPUT(PT_JSON, ci::Json)
			SET_LINKERPORTS_OUTPUT(PT_FEATURE, feature)
			SET_LINKERPORTS_OUTPUT(PT_BODYLIST, room::BodyRefList)
			SET_LINKERPORTS_OUTPUT(PT_BODY, room::BodyRef)
			SET_LINKERPORTS_OUTPUT(PT_VEC3, glm::vec3)
	default:
		break;
	}

	m_outputPorts.resize(0);
	m_outputPorts.push_back(m_internalOutputPort);

	m_inputPorts.resize(0);
	m_inputPorts.push_back(m_externalInputPort);

	m_internalInputPort = nullptr;
	m_externalOutputPort = nullptr;

	//m_name = toPort->getName();

	return m_internalOutputPort;
};

#define SET_LINKERPORTS_INPUT(port, type) case port:{  \
		auto externalOutputPort = OutputPort<type>::create(port, fromPort->getName());\
		m_externalOutputPort = externalOutputPort;\
		m_internalInputPort = InputPort<type>::create(port, fromPort->getName(), [externalOutputPort](type d) { externalOutputPort->send(d); });\
		break;\
	}

act::proc::PortBaseRef act::proc::LinkerProcNode::setInternalInputPort(PortBaseRef fromPort) {

	//only init new ports if the corresponding counter port is still not connected
	if (m_externalOutputPort->getType() != PT_GENERIC)
		return m_internalInputPort;

	switch (fromPort->getType()) {
		SET_LINKERPORTS_INPUT(PT_IMAGE, cv::UMat)
		SET_LINKERPORTS_INPUT(PT_NUMBER, float)
		SET_LINKERPORTS_INPUT(PT_TEXT, std::string)
		SET_LINKERPORTS_INPUT(PT_BOOL, bool)
		SET_LINKERPORTS_INPUT(PT_COLOR, ci::Color)
		SET_LINKERPORTS_INPUT(PT_JSON, ci::Json)
		SET_LINKERPORTS_INPUT(PT_FEATURE, feature)
		SET_LINKERPORTS_INPUT(PT_BODYLIST, room::BodyRefList)
		SET_LINKERPORTS_INPUT(PT_BODY, room::BodyRef)
		SET_LINKERPORTS_INPUT(PT_VEC3, glm::vec3)
	default:
		break;
	}

	m_outputPorts.resize(0);
	m_outputPorts.push_back(m_externalOutputPort);

	m_inputPorts.resize(0);
	m_inputPorts.push_back(m_internalInputPort);

	m_internalOutputPort = nullptr;
	m_externalInputPort = nullptr;

	//m_name = fromPort->getName();

	return m_internalInputPort;
};