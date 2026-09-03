
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

#pragma once

#include "ListenerBase.hpp"
#pragma warning( push )
#pragma warning( disable : 4081)
namespace act {
	namespace input {
		class KeyRawListener : public ListenerBase<KeyRawListener>
		{
		public:
			eventCall(keyRawDown, ci::app::KeyEvent);
			eventCall(keyRawUp, ci::app::KeyEvent);
		};

		class KeyListener : public ListenerBase<KeyListener>
		{
		public:
			eventCall(onKeyDown, ci::app::KeyEvent);
			eventCall(onKeyUp, ci::app::KeyEvent);
		};
	}
}
#pragma warning( pop )