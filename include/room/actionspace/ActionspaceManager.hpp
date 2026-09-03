
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

#include "cinder/audio/audio.h" 
#include "RoomNodeManagerBase.hpp"

#include <memory>
#include <vector>

#include "cinder/Capture.h"
#include "Port.hpp"


namespace act {
	namespace room {

		class ActionspaceManager : public RoomNodeManagerBase {
		public:
			ActionspaceManager();
			~ActionspaceManager();

			static	std::shared_ptr<ActionspaceManager> create() { return std::make_shared<ActionspaceManager>(); };

			void	setup() override;
			void	update() override;
			// void	draw() override;

			act::room::RoomNodeBaseRef	drawMenu() override;

			virtual ci::Json toJson();
			virtual void fromJson(ci::Json json);

			act::room::RoomNodeBaseRef addActionspace(std::string name, glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f));

		private:
			
			void refreshLists() override;

			
		};
		using ActionspaceManagerRef = std::shared_ptr<ActionspaceManager>;
	}
}