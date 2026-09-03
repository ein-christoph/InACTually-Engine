
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
#include "ListenerBase.hpp"
#include "MouseRawListener.hpp"


namespace act {
	namespace input {

		class MouseInput : public InputBase, MouseRawListener, MouseListener
		{
		public:
			MouseInput();
			virtual ~MouseInput();

			void update() override;

		protected:
			void mouseRawUp(ci::app::MouseEvent event)		override;
			void mouseRawDown(ci::app::MouseEvent event)		override;
			void mouseRawWheel(ci::app::MouseEvent event)	override;
			void mouseRawMove(ci::app::MouseEvent event)		override;
			void mouseRawDrag(ci::app::MouseEvent event)		override;
		};
	}
}