
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


namespace act {
	namespace input {

		class MouseRawListener;
		class KeyRawListener;
		class TouchRawListener;
		
		class InputRawListeners
		{
		public:
			virtual MouseRawListener*	getMouseRawListener() = 0;
			virtual KeyRawListener*		getKeyRawListener() = 0;
			virtual TouchRawListener*	getTouchRawListener() = 0;
		};


		class MouseListener;
		class KeyListener;
		class TouchListener;

		class InputListeners
		{
		public:
			virtual MouseListener*	getMouseListener() = 0;
			virtual KeyListener*	getKeyListener() = 0;
			virtual TouchListener*	getTouchListener() = 0;
		};
	}
}