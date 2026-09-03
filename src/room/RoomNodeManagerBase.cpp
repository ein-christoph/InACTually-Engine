
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
#include "RoomNodeManagerBase.hpp"

act::room::RoomNodeManagerBase::RoomNodeManagerBase(std::string name)
	: RoomNodeBase(name)
{
	CI_LOG_I("Creating " << name);
}

act::room::RoomNodeManagerBase::~RoomNodeManagerBase()
{
}

void act::room::RoomNodeManagerBase::update()
{
	for (auto&& node : m_nodes) {
		node->update();
	}
}

void act::room::RoomNodeManagerBase::draw()
{
	ci::gl::ScopedColor color;
	ci::gl::color(1.0f, 1.0f, 1.0f);
	ci::gl::ScopedMatrices push;

	for (auto&& node : m_nodes) {
		node->draw();
	}

}

void act::room::RoomNodeManagerBase::cleanUp()
{
	m_nodes.clear();
}

void act::room::RoomNodeManagerBase::addNode(RoomNodeBaseRef node)
{
	m_nodes.push_back(node);
}

act::room::RoomNodeBaseRef act::room::RoomNodeManagerBase::getNodeByUID(act::UID uid)
{
	for (auto&& node : m_nodes) {
		if (node->getUID() == uid)
			return node;
	}
	return nullptr;
}

bool act::room::RoomNodeManagerBase::hit(glm::vec3 pos)
{
	for (auto&& node : m_nodes) {
		if (node->hit(pos)) 
			return true;
	}
	return false;
}

bool act::room::RoomNodeManagerBase::hitRay(ci::Ray ray)
{
	for (auto&& node : m_nodes) {
		if (node->hitRay(ray))
			return true;
	}
	return false;
}

act::room::RoomNodeBaseRef act::room::RoomNodeManagerBase::getNodeAtPos(glm::vec3 pos)
{
	for (auto&& node : m_nodes) {
		if (node->hit(pos))
			return node;
	}
	return nullptr;
}

act::room::RoomNodeBaseRef act::room::RoomNodeManagerBase::getNodeOnRay(ci::Ray ray)
{
	float distance = FLT_MAX;
	RoomNodeBaseRef candidate;

	for (auto&& node : m_nodes) {
		if (node->hitRay(ray)) {
			float d = ci::distance(node->getPosition(), ray.getOrigin());
			if (distance > d) {
				distance = d;
				candidate = node;
			}
		}
	}

	return candidate;
}


bool act::room::RoomNodeManagerBase::removeNode(act::UID uid)
{
	size_t nsize = m_nodes.size();

	m_nodes.erase(		std::remove_if(m_nodes.begin(),		m_nodes.end(),		[&](RoomNodeBaseRef node) {	return node->getUID() == uid; }), m_nodes.end());
	refreshLists();

	return nsize != m_nodes.size();
}

void act::room::RoomNodeManagerBase::clear()
{
	m_nodes.clear();
	refreshLists();
}