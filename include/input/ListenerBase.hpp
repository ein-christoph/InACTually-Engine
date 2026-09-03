
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

#include "cinder/app/App.h"
#include <vector>
#include <algorithm>

#include "stddef.hpp"


/**
* @brief Listener (if overriden) or Observable (if not overriden)
*/
#define eventCall(function, paramType) virtual void function(paramType event) { OMP_FOR for (auto l : listener) l->function(event);	};


namespace act {
	namespace input {
		
		template <class T> class ListenerBase
		{
		public:
			/**
			* @brief if it is a Observable (if eventCall is not overriden) listeners can listen to events
			* @param listener which will be informed
			*/
			void addListener(T* listener) {
				this->listener.push_back(listener);
			}
			/**
			* @brief if it is a Observable (if eventCall is not overriden) listeners can be removed
			* @param listener to be remove
			*/
			void removeListener(T* listener) {
				this->listener.erase(std::remove(this->listener.begin(), this->listener.end(), listener), this->listener.end());
			}

			std::vector<T*> getListener() { return listener; }
		protected:
			std::vector<T*> listener;
		};
	}
}