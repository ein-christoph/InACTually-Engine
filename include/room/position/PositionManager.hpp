
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

#include "RoomNodeManagerBase.hpp"


namespace act {
	namespace room {

		class PositionManager : public RoomNodeManagerBase {
		public:
			PositionManager();//act::room::RoomManagers roomMgrs);
			~PositionManager();

			static	std::shared_ptr<PositionManager> create() { return std::make_shared<PositionManager>(); };

			void	setup() override;
			void	update() override;
			// void	draw() override;

			act::room::RoomNodeBaseRef drawMenu() override;

			virtual ci::Json toJson() override;
			virtual void fromJson(ci::Json json) override;

			act::room::RoomNodeBaseRef addPosition(glm::vec3 position);

		private:
	
			void refreshLists() override;
			
			

		};
		using PositionManagerRef = std::shared_ptr<PositionManager>;
	}
}