
/*
	InACTually
	> interactive theater for actual acts
	> this file is part of the "InACTually Engine", a MediaServer for driving all technology

	Copyright (c) 2021–2025 Lars Engeln, Fabian Töpfer
	Copyright (c) 2025 InACTually Community
	Licensed under the MIT License.
	See LICENSE file in the project root for full license information.

	This file is created and substantially modified: 2024

	contributors:
	Lars Engeln - mail@lars-engeln.de
*/


#pragma once

#include "cinder/Log.h"


namespace act {
	namespace util {

		class Logger
		{
		public:

			void static initialize();
			bool static m_isInitialized;
			
		private:
			
		};
	}
}