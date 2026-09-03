/*
	InACTually
	2021

	contributor:
	Lars Engeln - mail@lars-engeln.de

	interactive theater for actual acts
*/

#pragma once

#include "ListenerBase.hpp"


#pragma warning( push )
#pragma warning( disable : 4081)

namespace act {
	namespace input {

		class TouchRawListener : public ListenerBase<TouchRawListener>
		{
		public:
			eventCall(touchesRawBegan, ci::app::TouchEvent)
			eventCall(touchesRawMoved, ci::app::TouchEvent)
			eventCall(touchesRawEnded, ci::app::TouchEvent)
		};

		class TouchListener : public ListenerBase<TouchListener>
		{
		public:
			eventCall(onTouchesBegin, ci::app::TouchEvent)
			eventCall(onTouchesMove, ci::app::TouchEvent)
			eventCall(onTouchesEnd, ci::app::TouchEvent)
		};
	}
}
#pragma warning( pop )