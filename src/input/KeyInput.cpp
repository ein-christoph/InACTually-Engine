
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

#include "input\KeyInput.hpp"

using namespace act::input;

KeyInput::KeyInput()
{
}


KeyInput::~KeyInput()
{
}

void KeyInput::update() {};

void KeyInput::keyRawDown(ci::app::KeyEvent event) {
	onKeyDown(event);
};
void KeyInput::keyRawUp(ci::app::KeyEvent event) {
	onKeyUp(event);
};