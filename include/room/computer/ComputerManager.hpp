
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

#include "RoomNodeManagerBase.hpp"


namespace act {
	namespace room {

		class ComputerManager : public RoomNodeManagerBase {
		public:
			ComputerManager();//act::room::RoomManagers roomMgrs);
			~ComputerManager();

			static	std::shared_ptr<ComputerManager> create() { return std::make_shared<ComputerManager>(); };

			void	setup() override;
			void	update() override;
			// void	draw() override;

			act::room::RoomNodeBaseRef drawMenu() override;

			virtual ci::Json toJson() override;
			virtual void fromJson(ci::Json json) override;

		private:
	
			void refreshLists() override;
			
			act::room::RoomNodeBaseRef addDevice();

		};
		using ComputerManagerRef = std::shared_ptr<ComputerManager>;
	}
}