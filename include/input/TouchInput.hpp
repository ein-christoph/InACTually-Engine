
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

#include "stddef.hpp"

#include "InputBase.hpp"
#include "TouchRawListener.hpp"

#include <map>
 

namespace act {
	namespace input {

		class TouchInput : public InputBase, TouchRawListener, TouchListener
		{
		public:
			TouchInput();
			virtual ~TouchInput();

			void update() override;
			void drawDebug();

			void touchesRawBegan(ci::app::TouchEvent event) override;
			void touchesRawMoved(ci::app::TouchEvent event) override;
			void touchesRawEnded(ci::app::TouchEvent event) override;

		private: 
			std::map<uint32_t, glm::vec2> mActivePoints;
		};
	}
}