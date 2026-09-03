
/*
	InACTually
	> interactive theater for actual acts
	> this file is part of the "InACTually Engine", a MediaServer for driving all technology

	Copyright (c) 2021–2025 Lars Engeln, Fabian Töpfer
	Copyright (c) 2025 InACTually Community
	Licensed under the MIT License.
	See LICENSE file in the project root for full license information.

	This file is created and substantially modified: 2023

	contributors:
	Lars Engeln - mail@lars-engeln.de
*/

#pragma once

#include "stddef.hpp"
#include "ProcNodeBase.hpp"


namespace act {
	namespace proc {

		class ProcNodeRegistry
		{
		public:
			using nodeCreateFunc = std::shared_ptr<act::proc::ProcNodeBase>(*)();

		public:
			ProcNodeRegistry();

			bool add(std::string group, const std::string name, act::proc::ProcNodeRegistry::nodeCreateFunc createFunc);

			std::shared_ptr<act::proc::ProcNodeBase> create(const std::string& name);

			std::map<std::string, act::proc::ProcNodeRegistry::nodeCreateFunc>& getMap() {
				return m_nodeCreateMap;
			}
			std::map<std::string, std::vector<std::string>>& getGroups() {
				return m_groupMap;
			}

		private:
			std::map<std::string, act::proc::ProcNodeRegistry::nodeCreateFunc> m_nodeCreateMap; // nodeName -> ::create()
			std::map<std::string, std::vector<std::string>> m_groupMap; // groupName -> [nodeName]s
		};

	}
}