
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
#include "audio/HeadphoneRoomNode.hpp"


act::room::HeadphoneRoomNode::HeadphoneRoomNode(int channelL, int channelR, glm::vec3 position, float radius)
	: SpeakerRoomNode(channelL, position, radius, "headphones")
{
	m_isFixed = true;

	m_leftSpeaker	= SpeakerRoomNode::create(channelL, position + glm::vec3( 0.1f, 0.0f, 0.0f), radius);
	m_rightSpeaker	= SpeakerRoomNode::create(channelR, position + glm::vec3(-0.1f, 0.0f, 0.0f), radius);

	m_channelRouterNodeLeft = ci::audio::master()->makeNode(new ci::audio::ChannelRouterNode(ci::audio::Node::Format().channels(ci::audio::master()->getOutput()->getNumChannels())));
	m_channelRouterNodeRight = ci::audio::master()->makeNode(new ci::audio::ChannelRouterNode(ci::audio::Node::Format().channels(ci::audio::master()->getOutput()->getNumChannels())));

	getL()->getOut() >> m_channelRouterNodeLeft->route(0, m_leftSpeaker->getChannel(), 1) >> ci::audio::master()->getOutput();

	getR()->getOut() >> m_channelRouterNodeRight->route(0, m_rightSpeaker->getChannel(), 1) >> ci::audio::master()->getOutput();
}

act::room::HeadphoneRoomNode::~HeadphoneRoomNode()
{
}

void act::room::HeadphoneRoomNode::setup()
{
}

void act::room::HeadphoneRoomNode::update()
{
}

void act::room::HeadphoneRoomNode::draw()
{
	ci::gl::ScopedColor color(ci::Color(0.5f, 0.8f, 0.8f));

	if (getIsUnfolded()) {
		ci::gl::color(ci::Color(1.0f, 0.3f, 0.1f));
	}
	if (getIsEmphasized()) {
		ci::gl::color(ci::Color(1.0f, 1.0f, 1.0f));
		//setIsHovered(false);
	}

	// Draw the mesh.
	{
		//ci::gl::ScopedColor color(ci::Color::white());

		ci::gl::ScopedModelMatrix model;
		ci::gl::multModelMatrix(m_transform);

		//ci::gl::drawCube(getPosition(), glm::vec3(getRadius()) * glm::vec3(1.0f, 1.1f, 1.0f));
		m_mesh->draw();
	}
}

ci::audio::NodeRef act::room::HeadphoneRoomNode::getIn()
{
	return ci::audio::NodeRef();
}

ci::audio::NodeRef act::room::HeadphoneRoomNode::getOut()
{
	return ci::audio::NodeRef();
}

void act::room::HeadphoneRoomNode::setVolume(float volume, float rampDuration)
{
	m_leftSpeaker->setVolume(volume, rampDuration);
	m_rightSpeaker->setVolume(volume, rampDuration);
}

bool act::room::HeadphoneRoomNode::hit(glm::vec3 pos)
{
	return m_leftSpeaker->hit(pos) || m_rightSpeaker->hit(pos);
}

bool act::room::HeadphoneRoomNode::hitRay(ci::Ray ray)
{
	return m_leftSpeaker->hitRay(ray) || m_rightSpeaker->hitRay(ray);
}

void act::room::HeadphoneRoomNode::setIsHovered(bool hovered)
{
	//m_isHovered = hovered; 
	m_leftSpeaker->setIsSelected(hovered); 
	m_rightSpeaker->setIsSelected(hovered);
}

void act::room::HeadphoneRoomNode::setIsSelected(bool selected)
{
	//m_isSelected = selected; 
	m_leftSpeaker->setIsSelected(selected); 
	m_rightSpeaker->setIsSelected(selected);
}

void act::room::HeadphoneRoomNode::disconnectExternals()
{
	m_leftSpeaker->disconnectExternals();
	m_rightSpeaker->disconnectExternals();
}

void act::room::HeadphoneRoomNode::setupAudioNodes()
{
	m_leftSpeaker = SpeakerRoomNode::create(m_leftSpeaker->getChannel(), m_leftSpeaker->getPosition(), m_leftSpeaker->getRadius());
	m_rightSpeaker = SpeakerRoomNode::create(m_rightSpeaker->getChannel(), m_rightSpeaker->getPosition(), m_rightSpeaker->getRadius());
	m_channelRouterNodeLeft = ci::audio::master()->makeNode(new ci::audio::ChannelRouterNode(ci::audio::Node::Format().channels(ci::audio::master()->getOutput()->getNumChannels())));
	m_channelRouterNodeRight = ci::audio::master()->makeNode(new ci::audio::ChannelRouterNode(ci::audio::Node::Format().channels(ci::audio::master()->getOutput()->getNumChannels())));
	getL()->getOut() >> m_channelRouterNodeLeft->route(0, m_leftSpeaker->getChannel(), 1) >> ci::audio::master()->getOutput();
	getR()->getOut() >> m_channelRouterNodeRight->route(0, m_rightSpeaker->getChannel(), 1) >> ci::audio::master()->getOutput();
}

void act::room::HeadphoneRoomNode::locate(bool isLocating)
{
	m_leftSpeaker->locate(isLocating);
	m_rightSpeaker->locate(isLocating);
}

void act::room::HeadphoneRoomNode::calibrate(std::string type, float duration, float volume)
{
	m_leftSpeaker->calibrate(type, duration, volume);
	m_rightSpeaker->calibrate(type, duration, volume);
}

/*
void act::room::HeadphoneRoomNode::setEQFilter(cv::UMat filterLeft, cv::UMat filterRight)
{
	m_leftSpeaker->setEQFilter(filterLeft);
	m_rightSpeaker->setEQFilter(filterRight);
}

void act::room::HeadphoneRoomNode::resetEQFilter()
{
	m_leftSpeaker->resetEQFilter();
	m_rightSpeaker->resetEQFilter();
}
*/

void act::room::HeadphoneRoomNode::drawSpecificSettings()
{
	if (ImGui::CollapsingHeader("Left")) {
		m_leftSpeaker->drawSpecificSettings();
	}
	if (ImGui::CollapsingHeader("Right")) {
		m_rightSpeaker->drawSpecificSettings();
	}
}