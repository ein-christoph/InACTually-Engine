
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

		class MouseRawListener : public ListenerBase<MouseRawListener>
		{
		public:
			eventCall(mouseRawMove, ci::app::MouseEvent);
			eventCall(mouseRawDown, ci::app::MouseEvent);
			eventCall(mouseRawDrag, ci::app::MouseEvent);
			eventCall(mouseRawUp, ci::app::MouseEvent);
			eventCall(mouseRawWheel, ci::app::MouseEvent);
		};

		class MouseListener : public ListenerBase<MouseListener>
		{
		public:
			eventCall(onMouseMove, ci::app::MouseEvent);
			eventCall(onMouseDown, ci::app::MouseEvent);
			eventCall(onMouseDrag, ci::app::MouseEvent);
			eventCall(onMouseUp, ci::app::MouseEvent);
			eventCall(onMouseWheel, ci::app::MouseEvent);
		};
	}
}
#pragma warning( pop )