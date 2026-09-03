
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

#include <cinder/app/App.h>


namespace act {

	class DrawableBase
	{
	public:
		DrawableBase() {};
		virtual ~DrawableBase() {};

		virtual void draw() = 0;
		virtual void update() = 0;
			
	private:

	}; using DrawableBaseRef = std::shared_ptr<DrawableBase>;
	
}