
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
#include "audio/MicrophoneRoomNode.hpp"

#include "cinder/audio/Param.h"



act::room::MicrophoneRoomNode::MicrophoneRoomNode(int channel, glm::vec3 position, float radius, std::string name)
	: AudioRoomNodeBase("microphone", position, glm::vec3(0.0f), radius), m_channel(channel)
{
	m_isFixed = false;

	setTriMesh(ci::TriMesh::create(ci::geom::Capsule()));

	setupAudioNodes();
}

act::room::MicrophoneRoomNode::~MicrophoneRoomNode()
{
}

void act::room::MicrophoneRoomNode::setup()
{
}

void act::room::MicrophoneRoomNode::update()
{
}

void act::room::MicrophoneRoomNode::draw()
{
	ci::gl::ScopedColor color(ci::Color(0.5f, 0.5f, 0.5f));

	enableStatusColor();

	// Draw the mesh.
	{
		//ci::gl::ScopedColor color(ci::Color::white());

		ci::gl::ScopedModelMatrix model;
		ci::gl::multModelMatrix(m_transform);

		//ci::gl::drawCube(getPosition(), glm::vec3(getRadius()) * glm::vec3(1.0f, 1.1f, 1.0f));
		m_mesh->draw();
	}
}

ci::audio::NodeRef act::room::MicrophoneRoomNode::getIn()
{
	return m_add;
}

ci::audio::NodeRef act::room::MicrophoneRoomNode::getOut()
{
	return m_gain;
}

void act::room::MicrophoneRoomNode::setupAudioNodes()
{
	auto ctx = ci::audio::Context::master();

	m_gain = ctx->makeNode(new ci::audio::GainNode(ci::audio::decibelToLinear(m_volume)));

	m_add = ctx->makeNode(new ci::audio::AddNode());
	//m_eq  = ctx->makeNode(new EqualizerNode(ci::audio::Node::Format().channels(1)));

	disconnectExternals();
}

void act::room::MicrophoneRoomNode::disconnectExternals()
{
	getOut()->disconnectAll();
	getIn() >> /*m_eq >>*/ getOut();
}


void act::room::MicrophoneRoomNode::calibrate(float duration, float volume)
{

}

void act::room::MicrophoneRoomNode::drawSpecificSettings()
{
	if (ImGui::InputInt("Channel", &m_channel, 1, 1)) {

	}
}


ci::Json act::room::MicrophoneRoomNode::toParams() {
	ci::Json json = ci::Json::object();
	json["channel"]	= m_channel;
	json["volume"]	= m_volume.value();
	return json;
}

void act::room::MicrophoneRoomNode::fromParams(ci::Json json) {
	util::setValueFromJson(json, "channel", m_channel);
	float volume = 100;
	if (util::setValueFromJson(json, "volume", volume)) {
		setVolume(volume, 0.1f);
	};
}