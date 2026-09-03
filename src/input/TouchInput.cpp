
/*
	InACTually
	> interactive theater for actual acts
	> this file is part of the "InACTually Engine", a MediaServer for driving all technology

	Copyright (c) 2021–2025 Lars Engeln, Fabian Töpfer
	Copyright (c) 2025 InACTually Community
	Licensed under the MIT License.
	See LICENSE file in the project root for full license information.

	This file is created and substantially modified: 2021

	contributors:
	Lars Engeln - mail@lars-engeln.de
*/

#include "input\TouchInput.hpp"

#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"

#include "stddef.hpp"
#include <sstream>

using namespace act::input;

TouchInput::TouchInput()
{
}


TouchInput::~TouchInput()
{
}

void TouchInput::update() {

}

void TouchInput::drawDebug() {
	ci::gl::color(ci::ColorA(0.8f, 0.2f, 0.2f, 0.7f));
	for (const auto &touch : mActivePoints) {
		ci::gl::drawSolidCircle(touch.second, 40);
		std::stringstream id;
		id << touch.first << " at " << touch.second.x << ", " << touch.second.y;
		ci::gl::drawString(id.str(), touch.second + glm::vec2(-40, -60), ci::ColorA(0.8f, 0.2f, 0.2f, 0.7f));
		//ci::gl::lineWidth(5);
		//ci::gl::drawLine(touch.second, touch.second + (normalize(touch.second - touch.getPrevPos()) * glm::vec2(60)));
	}
}

void TouchInput::touchesRawBegan(ci::app::TouchEvent event) {
	for (const auto &touch : event.getTouches()) {
		mActivePoints.insert(std::make_pair(touch.getId(), touch.getPos()));
	}
	if (event.getTouches().size() > 0) {
		onTouchesBegin(event);
	}
}

void TouchInput::touchesRawMoved(ci::app::TouchEvent event) {
	for (const auto &touch : event.getTouches()) {
		mActivePoints[touch.getId()] = touch.getPos();
	}
	if (event.getTouches().size() > 0) {
		onTouchesMove(event);
	}
}

void TouchInput::touchesRawEnded(ci::app::TouchEvent event) {
	for (const auto &touch : event.getTouches()) {
		mActivePoints.erase(touch.getId());
	}
	if (event.getTouches().size() > 0) {
		onTouchesEnd(event);
	}
}
