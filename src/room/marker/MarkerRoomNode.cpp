
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
#include "marker/MarkerRoomNode.hpp"

#include <opencv2/objdetect/aruco_detector.hpp>

act::room::MarkerRoomNode::MarkerRoomNode(int id, act::UID replyUID)
	: RoomNodeBase("marker"/*std::to_string(id)*/, glm::vec3(0, 0, 0), glm::vec3(0, 0, 0), 0.075f, replyUID)
{
	m_id = id;
	doSmoothing(true);

	setTriMesh(ci::TriMesh::create(ci::geom::Plane()));
	setMarkerImage();
}

act::room::MarkerRoomNode::~MarkerRoomNode()
{
}

void act::room::MarkerRoomNode::setup()
{
}

void act::room::MarkerRoomNode::update()
{
}

void act::room::MarkerRoomNode::draw()
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

		if (m_markerTexture) {
			ci::gl::ScopedColor color(ci::Color(1.0f, 1.0f, 1.0f));
			ci::gl::rotate(glm::half_pi<float>(), 1.0f, 0.0f, 0.0f);
			ci::gl::translate(0.0f, 0.0f, -0.001f);
			ci::Rectf drawRect(-5.0f * m_markerWidth, -5.0f * m_markerWidth, 5.0f *m_markerWidth, 5.0f *m_markerWidth);
			ci::gl::draw(m_markerTexture, drawRect);
		}
	}
}

void act::room::MarkerRoomNode::drawSpecificSettings()
{
	if (ImGui::InputInt("Marker ID", &m_id)) // carefull, a markerRoomNode is identified by a detected marker-ID
		setMarkerImage();
	ImGui::Checkbox("dynamic", &m_dynamic);

}

ci::Json act::room::MarkerRoomNode::toParams()
{
	return ci::Json();
}

void act::room::MarkerRoomNode::fromParams(ci::Json json)
{
}

void act::room::MarkerRoomNode::setMarkerImage() {
	cv::Mat markerImage;
	cv::aruco::Dictionary dictionary = cv::aruco::getPredefinedDictionary(cv::aruco::DICT_4X4_250);
	cv::aruco::generateImageMarker(dictionary, m_id, m_markerWidth * 100.0f, markerImage, 1);

	cv::cvtColor(markerImage, markerImage, cv::COLOR_GRAY2BGR);

	m_markerTexture = ci::gl::Texture::create(ci::fromOcv(markerImage));
}
