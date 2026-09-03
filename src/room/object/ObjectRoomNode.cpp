
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
#include "object/ObjectRoomNode.hpp"

act::room::ObjectRoomNode::ObjectRoomNode(int id, std::string name, act::UID replyUID)
	: RoomNodeBase("object", glm::vec3(0, 0, 0), glm::vec3(0, 0, 0), 0.075f, replyUID), m_id(id)
{

	doSmoothing(true);

	setTriMesh(ci::TriMesh::create(ci::geom::Plane()));
	setObjectImage();
}

act::room::ObjectRoomNode::~ObjectRoomNode()
{
}

void act::room::ObjectRoomNode::setup()
{
}

void act::room::ObjectRoomNode::update()
{
}

void act::room::ObjectRoomNode::draw()
{
	ci::gl::ScopedColor color(ci::Color(0.6f, 0.6f, 0.6f));

	enableStatusColor();

	// Draw the mesh.
	{
		ci::gl::ScopedModelMatrix model;
		ci::gl::multModelMatrix(m_transform);

		util::drawCoords();

		ci::gl::rotate(glm::half_pi<float>(), 1.0f, 0.0f, 0.0f);
		m_mesh->draw();

		if (m_objectTexture) {
			ci::gl::ScopedColor color(ci::Color(1.0f, 1.0f, 1.0f));
			ci::gl::rotate(glm::half_pi<float>(), 1.0f, 0.0f, 0.0f);
			ci::gl::translate(0.0f, 0.0f, -0.001f);
			ci::Rectf drawRect(-5.0f * m_objectWidth, -5.0f * m_objectWidth, 5.0f * m_objectWidth, 5.0f * m_objectWidth);
			ci::gl::draw(m_objectTexture, drawRect);
		}
	}
}

void act::room::ObjectRoomNode::drawSpecificSettings()
{
	ImGui::Checkbox("dynamic", &m_dynamic);

}

ci::Json act::room::ObjectRoomNode::toParams()
{
	return ci::Json();
}

void act::room::ObjectRoomNode::fromParams(ci::Json json)
{
}

void act::room::ObjectRoomNode::setObjectImage() {
	

	//m_objectTexture = ci::gl::Texture::create(ci::fromOcv(image));
}
