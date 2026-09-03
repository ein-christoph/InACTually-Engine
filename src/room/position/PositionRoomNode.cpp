
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
#include "position/PositionRoomNode.hpp"


act::room::PositionRoomNode::PositionRoomNode(std::string name, glm::vec3 position, act::UID replyUID)
	: RoomNodeBase("position", position, glm::vec3(0.0f), 0.2f, replyUID)
{

	setPosition(glm::vec3(0.0f, 0.0f, 0.0f));
	setRotation(glm::vec3(0.0f, 0.0f, 0.0f));

	addControlPoint(glm::vec3(0));
	m_position = glm::vec3(0);
	m_evaluatedPosition = glm::vec3(0);

	m_isHighlighted = false;
	m_degree = 1;

	evaluatePosition(0.0f);
}

act::room::PositionRoomNode::~PositionRoomNode()
{

}

void act::room::PositionRoomNode::setup()
{
	
}

void act::room::PositionRoomNode::update()
{

}

void act::room::PositionRoomNode::draw()
{
	ci::gl::ScopedColor color;
	enableStatusColor();

	ci::gl::pushMatrices();
	ci::gl::translate(m_position);
	ci::gl::rotate(m_rotation);
	ci::gl::drawSphere(glm::vec3(0.0f), 0.2f, 4);

	if (getIsEmphasized() || getIsUnfolded()) {
		glm::vec3 priorPt = glm::vec3(0.0f);
		if (m_controlPoints.size() >= 1) {
			priorPt = m_controlPoints[0];
		}
		ci::gl::ScopedLineWidth(5.0f);
		for (auto pt : m_controlPoints) {
			ci::gl::pushMatrices();
			ci::gl::translate(pt);
			ci::gl::drawSphere(glm::vec3(0.0f), 0.075f, 4);
			ci::gl::popMatrices();

			ci::gl::drawLine(priorPt, pt);
			priorPt = pt;
		}
		if(m_points.size() != 0)
			priorPt = m_points[0];
		for (auto pt : m_points) {
			ci::gl::drawLine(priorPt, pt);
			priorPt = pt;
		}

	}
	else {
		ci::gl::drawLine(glm::vec3(0.0f), m_evaluatedPosition);
	}

	ci::gl::pushMatrices();
	ci::gl::translate(m_evaluatedPosition);
	ci::gl::drawSphere(glm::vec3(0.0f), 0.06f, 16);
	ci::gl::popMatrices();

	ci::gl::popMatrices();
}

void act::room::PositionRoomNode::drawSpecificSettings()
{
	bool loop = m_isLooping;
	if (ImGui::Checkbox("loop", &loop)) {
		setIsLooping(loop);
	}
}

ci::Json act::room::PositionRoomNode::toParams()
{
	ci::Json json	= ci::Json::object();
	json["degree"]	= m_degree;
	json["loop"]	= m_isLooping;

	ci::Json points = ci::Json::array();
	for (auto&& pt : m_controlPoints) {
		points.push_back(pt);
	}
	json["controlPoints"] = points;

	return json;
}

void act::room::PositionRoomNode::fromParams(ci::Json json)
{
	m_controlPoints.resize(0);
	for (auto pt : json["controlPoints"]) {
		addControlPoint(pt);
	}

	util::setValueFromJson(json, "degree", m_degree);
	util::setValueFromJson(json, "loop", m_isLooping);
	setDegree(getDegree());
	updateSpline();
}

int act::room::PositionRoomNode::addControlPoint(glm::vec3 position)
{
	m_controlPoints.push_back(position);
	setDegree(getDegree());
	updateSpline();
	return m_controlPoints.size() - 1;
}

void act::room::PositionRoomNode::removeControlPoint(int index)
{
	if(index < m_controlPoints.size())
		m_controlPoints.erase(m_controlPoints.begin() + index);
	setDegree(getDegree());
	updateSpline();
}

glm::vec3 act::room::PositionRoomNode::getControlPoint(int index)
{
	if(index < m_controlPoints.size())
		return m_controlPoints[index];

	return glm::vec3(0.0f);
}

glm::vec3 act::room::PositionRoomNode::getLastControlPoint()
{
	if (m_controlPoints.size() == 0)
		return glm::vec3(0.0f);

	return m_controlPoints[m_controlPoints.size() - 1];
}

void act::room::PositionRoomNode::setControlPoint(int index, glm::vec3 position)
{
	if (index >= m_controlPoints.size())
		return;

	m_controlPoints[index] = position;
	updateSpline();
}

int act::room::PositionRoomNode::getIndexOfControlPointAt(glm::vec3 position)
{
	for (int i = 0; i < m_controlPoints.size(); i++) {
		if (distance2(position, m_controlPoints[i]) < 0.001f)
			return i;
	}

	return -1;
}

void act::room::PositionRoomNode::setIsLooping(bool isLooping)
{
	if (m_controlPoints.size() == 0)
		return;

	if (isLooping && !m_isLooping) {
		m_isLooping = true;
		updateSpline();
	}
	else if (!isLooping && m_isLooping) {
		m_isLooping = false;
		updateSpline();
	}
}

int act::room::PositionRoomNode::setDegree(int degree)
{
	if (degree < 1)
		m_degree = 1;
	else if (degree >= m_controlPoints.size() - 1)
		m_degree = m_controlPoints.size() - 1;
	else m_degree = degree;
	updateSpline();

	return m_degree;
}

glm::vec3 act::room::PositionRoomNode::evaluatePosition(float t)
{
	if (m_controlPoints.size() <= 1)
		return m_position;

	if (abs(t) == NAN)
		return m_position;

	m_evaluatedPosition = m_spline.getPosition(t);
	return m_evaluatedPosition + m_position;
}

void act::room::PositionRoomNode::updateSpline()
{
	if (m_controlPoints.size() < 2)
		return;

	m_spline = ci::BSpline3f(m_controlPoints, m_degree, m_isLooping, !m_isLooping);
	m_points.resize(0);
	for (float t = 0.00f; t < 1.0f; t += 0.01f) {
		m_points.push_back(m_spline.getPosition(t));
	}

	evaluatePosition(m_t);
}

