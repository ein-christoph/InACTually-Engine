
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

#include "cinder/app/Event.h"
#include "input/InputListeners.hpp"
#include "input/MouseRawListener.hpp"
//#include "input/KeyRawListener.hpp"
//#include "input/TouchRawListener.hpp"
#include "cinder/CinderImGui.h"


namespace act {
	/**
	* @brief interactionHelper for ImGui
	*
	*/
	namespace ia {

		enum MouseButton {
			MB_NONE = -1,
			MB_BTNLEFT = 0,
			MB_BTNRIGHT = 1,
			MB_BTNMIDDLE
		};
		struct MouseEvent {
			glm::vec2 position;
			MouseButton button;
		};

		/**
		 * @brief The InteractionHelper throws events from ImGui states
		 * 
		 */
		class InteractionHelper : public act::input::MouseListener
		{
		public:
			InteractionHelper();
			~InteractionHelper();

			static std::shared_ptr<InteractionHelper> create() { return std::make_shared<InteractionHelper>(); };

			void evaluate();

			virtual input::MouseListener* getMouseListener();
			//virtual input::KeyListener*	  getKeyListener();
			//virtual input::TouchListener* getTouchListener();

			glm::vec2 getNormalizedMousePos() { return m_mousePosNorm; };

		private:

			bool evaluateMouse();
			bool evaluateMouseButton(MouseButton btn);

			ImVec2		m_mousePos;
			glm::vec2	m_mousePosNorm;
			bool		m_isDragging;
			float		m_wheelIncrement;
		};
		typedef	std::shared_ptr<InteractionHelper> InteractionHelperRef;
	};
};