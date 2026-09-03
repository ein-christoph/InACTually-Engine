
/*
	InACTually
	> interactive theater for actual acts
	> this file is part of the "InACTually Engine", a MediaServer for driving all technology

	Copyright (c) 2025 InACTually Community
	Licensed under the MIT License.
	See LICENSE file in the project root for full license information.

	This file is created and substantially modified: 2025

	contributors:
	Lars Engeln - mail@lars-engeln.de
*/

#pragma once

#include "DrawableBase.hpp"
#include <cinder/app/App.h>


namespace act {
	namespace room {

		class CallbackDrawable : public DrawableBase
		{
		public:
			CallbackDrawable() {};
			virtual ~CallbackDrawable() {};

			static std::shared_ptr<CallbackDrawable> create() { return std::make_shared<CallbackDrawable>(); };

			virtual void update()	override { m_updateCallback(); };
			virtual void draw()		override { m_drawCallback(); };

			void setUpdateCallback(std::function<void()> callback)	{ m_updateCallback = callback; };
			void setDrawCallback(std::function<void()> callback)	{ m_drawCallback = callback; };

		private:
			std::function<void()> m_updateCallback	= []() {};
			std::function<void()> m_drawCallback	= []() {};

		}; using CallbackDrawableRef = std::shared_ptr<CallbackDrawable>;
		
	}
}