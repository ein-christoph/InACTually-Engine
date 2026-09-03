
/*
	InACTually
	> interactive theater for actual acts
	> this file is part of the "InACTually Engine", a MediaServer for driving all technology

	Copyright (c) 2021–2025 Lars Engeln, Fabian Töpfer
	Copyright (c) 2025 InACTually Community
	Licensed under the MIT License.
	See LICENSE file in the project root for full license information.

	This file is created and substantially modified: 2021-2025

	contributors:
	Lars Engeln - mail@lars-engeln.de
*/

#include "roompch.hpp"
#include "dmx/DimmerRoomNode.hpp"

#include "RGBAWHelper.h"

act::room::DimmerRoomNode::DimmerRoomNode(DMXProRef dmxInterface, ci::Json description, std::string name, int startAddress, glm::vec3 position, glm::vec3 rotation, float radius, act::UID replyUID)
	: DMXRoomNodeBase(dmxInterface, description, startAddress), RoomNodeBase("dimmer", position, rotation, radius, replyUID)
{
	 
	setPosition(glm::vec3(1.0f, 1.0f, 0.0f));
	setRotation(glm::vec3(0.0f, 0.0f, 0.0f));
	lookAt(glm::vec3(5.0f, 2.0f, 5.0f));
}

act::room::DimmerRoomNode::~DimmerRoomNode()
{
	
}

void act::room::DimmerRoomNode::setup()
{
	
}

void act::room::DimmerRoomNode::update()
{
	
}

void act::room::DimmerRoomNode::draw()
{
	ci::gl::ScopedColor color;
	
	ci::gl::pushMatrices();
		ci::gl::translate(m_position);

		ci::gl::color(ci::Color::white());

		ci::gl::color(ci::Color(1.0f, 1.0f, 0.0f));

		//glm::vec3 v = m_cameraPersp.getViewDirection();
		//ci::gl::drawVector(glm::vec3(0.0f), normalize(glm::vec3(v.x, 0.0f, v.z)));
		//ci::gl::drawVector(glm::vec3(0.0f), normalize(glm::vec3(0.0f, v.y, v.z)));

		enableStatusColor();

		ci::gl::drawCube(glm::vec3(0.0f,0.0f,0.0f), glm::vec3(1.0f, 1.0f, 1.0f));
		 
	ci::gl::popMatrices();

	if (m_isLookingAt) {
		glEnable(GL_BLEND); 
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		//TODO: change to project color
		
		ci::gl::color(util::Design::primaryColor());
		ci::gl::drawSphere(m_lookAt, 0.25f);
	}
	 
}

void act::room::DimmerRoomNode::cleanUp()
{
	setDimmer(0.0f);
}

void act::room::DimmerRoomNode::drawSpecificSettings()
{
  

}

ci::Json act::room::DimmerRoomNode::toParams()
{
	ci::Json json = ci::Json::object();
	json["startAddress"] = getStartAddress();
	json["fixtureName"] = getFixtureName();
	json["dimmer"] = m_dimmer.getValue();
 
	return json;
}

void act::room::DimmerRoomNode::fromParams(ci::Json json)
{
	util::setValueFromJson(json, "dimmer", m_dimmer.value);
	
	int startAdress = 0;
	if (util::setValueFromJson(json, "startAddress", startAdress)) {
		setStartAddress(startAdress);
	}
}
 
void act::room::DimmerRoomNode::setDimmer(float dim)
{
	m_dimmer.setValue(dim);
	setValue("dimmer", m_dimmer.getValue() * 255);
}
 
