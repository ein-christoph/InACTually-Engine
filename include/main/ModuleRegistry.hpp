
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

#include "modules/ModuleBase.hpp"
#include <vector>


namespace act {
	namespace mod {

		class ModuleRegistry {
		public:
			~ModuleRegistry() {};
			static void useModule(act::mod::ModuleBaseRef module) { reg_modules.push_back(module); }
			static act::mod::ModuleBaseRef getModuleByName(std::string name) {
				for (auto&& module : reg_modules) {
					if (module->getName() == name)
						return module;
				}
				return nullptr;
			}
		protected:
			ModuleRegistry() {};
			static std::vector<act::mod::ModuleBaseRef> reg_modules;
		};
	}
}
