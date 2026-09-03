
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

#include "input\MouseInput.hpp"

using namespace act::input;

MouseInput::MouseInput()
{
}

MouseInput::~MouseInput()
{
}

void MouseInput::update() {
}

void MouseInput::mouseRawMove(ci::app::MouseEvent event) {
	onMouseMove(event);
}

void MouseInput::mouseRawUp(ci::app::MouseEvent event) {
	onMouseUp(event);
}

void MouseInput::mouseRawDrag(ci::app::MouseEvent event) {
	onMouseDrag(event);
}

void MouseInput::mouseRawDown(ci::app::MouseEvent event)
{
	onMouseDown(event);
}

void MouseInput::mouseRawWheel(ci::app::MouseEvent event) {
	onMouseWheel(event);
}
