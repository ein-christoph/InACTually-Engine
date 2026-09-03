
/*
	InACTually
	> interactive theater for actual acts
	> this file is part of the "InACTually Engine", a MediaServer for driving all technology

	Copyright (c) 2021–2025 Lars Engeln, Fabian Töpfer
	Copyright (c) 2025 InACTually Community
	Licensed under the MIT License.
	See LICENSE file in the project root for full license information.

	This file is created and substantially modified: 2025

	contributors:
	Lars Engeln - mail@lars-engeln.de
*/

#include "roompch.hpp"
#include "body/BodyRoomNode.hpp"


act::room::BodyRoomNode::BodyRoomNode(std::string name, glm::vec3 position, glm::vec3 rotation, float radius, act::UID replyUID)
	: RoomNodeBase("body", position, rotation, radius, replyUID)
{
	m_body = Body::create();
}

act::room::BodyRoomNode::~BodyRoomNode()
{

}

void act::room::BodyRoomNode::setup()
{
}

void act::room::BodyRoomNode::update()
{
	
	
}

void act::room::BodyRoomNode::draw()
{
	//enableStatusColor(); 

	ci::gl::pushMatrices();
	//ci::gl::translate(m_position);
	//ci::gl::rotate(m_rotation);
	ci::gl::drawCube(glm::vec3(0.0f), glm::vec3(0.1f, 0.075f, 0.075f));
	auto color = ci::ColorA(0.9f, 0.9f, 0.9f, 0.85f);
	float sphereRadius = 0.075f;

	for (auto&& joint : m_body->joints)
	{
		glm::vec3 pos = joint->position;
		switch (joint->confidenceLevel) {
		case act::room::BJC_NONE:
			ci::gl::color(ci::ColorA(color.r, color.g, color.b, 0.4f));
			ci::gl::lineWidth(2);
			break;
		case act::room::BJC_LOW:
			ci::gl::color(ci::ColorA(color.r, color.g, color.b, 0.6f));
			ci::gl::lineWidth(4);
			break;
		case act::room::BJC_HIGH:
			ci::gl::color(color);
			ci::gl::lineWidth(4);
			break;
		default:
			break;
		}

		if (joint->type == act::room::BJT_HEAD)
			ci::gl::drawSphere(pos, sphereRadius * 4.0f / 3.0f, 8);
		else
			ci::gl::drawSphere(pos, sphereRadius, 8);

		glm::vec3 parent = m_body->joints[act::room::bodyJointParentLookUp[joint->type]]->position;

		ci::gl::drawLine(pos, parent);

		// drawVector only if debugging
		/*
		ci::gl::pushMatrices();
		ci::gl::translate(pos);
		ci::gl::rotate(joint->orientation);
		ci::gl::drawCoordinateFrame(0.2f);
		ci::gl::popMatrices();
		*/
	}

	util::drawCoords();

	ci::gl::popMatrices();
}

void act::room::BodyRoomNode::drawSpecificSettings()
{
	
}

ci::Json act::room::BodyRoomNode::toParams()
{
	ci::Json params = ci::Json::object();
	params["body"] = m_body->toJson();
	return params;
}

void act::room::BodyRoomNode::fromParams(ci::Json params)
{
	if (params.contains("body"))
		m_body->fromJson(params["body"]);
}

void act::room::BodyRoomNode::setBody(BodyRef body)
{
	m_body = body;

	publishParam("body", body->toJson());
}
