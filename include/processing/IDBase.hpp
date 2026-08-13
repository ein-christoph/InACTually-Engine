
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


using namespace ci;
using namespace ci::app;

namespace act {
	namespace proc {

		class IDBase
		{
		public:
			IDBase() { m_id = nextID++; };
			virtual ~IDBase() {};

			unsigned int getID() { return m_id; }

			static unsigned int getNextID() { return nextID; }
			static void resetNextID() { nextID = 1; }

		protected:
			unsigned int m_id;

		private:
			static unsigned int nextID;
		};

	}
}