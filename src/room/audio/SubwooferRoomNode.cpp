
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

#include "roompch.hpp"
#include "audio/SubwooferRoomNode.hpp"


act::room::SubwooferRoomNode::SubwooferRoomNode(int channel, glm::vec3 position, float radius)
	: SpeakerRoomNode(channel, position, radius, "subwoofer")
{
	setTriMesh(ci::TriMesh::create(ci::geom::Cylinder()));
}

act::room::SubwooferRoomNode::~SubwooferRoomNode()
{
}

void act::room::SubwooferRoomNode::draw()
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


void act::room::SubwooferRoomNode::drawSpecificSettings()
{
	if (ImGui::InputInt("Channel", &m_channel, 1, 1)) {

	}
	
	ImGui::Separator();
	if (ImGui::DragFloat("NoiseVolume", &m_noiseVolume.value(), 0.1f, 0.0f, 120.0f)) {
		m_noiseGain->setValue(ci::audio::decibelToLinear(m_noiseVolume));
	}
	if (ImGui::SmallButton("locate")) {
		m_noise->setEnabled(!m_noise->isEnabled());
	}

}