
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

#include "InputListeners.hpp"
#include "input/MouseRawListener.hpp"
#include "input/KeyRawListener.hpp"
#include "input/TouchRawListener.hpp"

#include "InteractionManager.hpp"
#include <thread> 


namespace act {
	/**
	* @brief input contains the handling of the raw input from mouse (MouseListener), keyboard (KeyListener) and touch (TouchListener)
	*
	*/
	namespace input {

		class MouseInput;
		class KeyInput;	
		class TouchInput;

		class InputManager : public InputRawListeners, public MouseListener, public KeyListener, public TouchListener
		{
		public:
			InputManager(ia::InteractionMgrRef iaMgr);
			virtual ~InputManager();

			void update();
			void drawDebug();

			MouseRawListener*	getMouseRawListener()	override;
			KeyRawListener*		getKeyRawListener()		override;
			TouchRawListener*	getTouchRawListener()	override;

		private:
			ia::InteractionMgrRef		interactionMgr;

			std::shared_ptr<MouseInput>		mouseInput;
			std::shared_ptr<KeyInput>		keyInput;
			std::shared_ptr<TouchInput>		touchInput;

			void onKeyDown(ci::app::KeyEvent event)					override;
			void onKeyUp(ci::app::KeyEvent event)					override;
			void onMouseUp(ci::app::MouseEvent event)				override;
			void onMouseDown(ci::app::MouseEvent event)				override;
			void onMouseWheel(ci::app::MouseEvent event)				override;
			void onMouseMove(ci::app::MouseEvent event)				override;
			void onMouseDrag(ci::app::MouseEvent event)				override;
			void onTouchesBegin(ci::app::TouchEvent event)	override;
			void onTouchesMove(ci::app::TouchEvent event)	override;
			void onTouchesEnd(ci::app::TouchEvent event)	override;

			std::thread updateThread;
			int			ups;
			bool		doUpdates = true;	
		};

		typedef std::shared_ptr<InputManager> InputManagerRef;
	}
}