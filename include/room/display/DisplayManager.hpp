
/*
	InACTually
	> interactive theater for actual acts
	> this file is part of the "InACTually Engine", a MediaServer for driving all technology

	Copyright (c) 2021–2025 Lars Engeln, Fabian Töpfer
	Copyright (c) 2025 InACTually Community
	Licensed under the MIT License.
	See LICENSE file in the project root for full license information.

	This file is created and substantially modified: 2022-2023

	contributors:
	Lars Engeln - mail@lars-engeln.de
*/

#pragma once

#include "RoomNodeManagerBase.hpp"


namespace act {
	namespace room {

		class DisplayManager : public RoomNodeManagerBase {
		public:
			DisplayManager();
			~DisplayManager();

			static	std::shared_ptr<DisplayManager> create() { return std::make_shared<DisplayManager>(); };

			void	setup() override;
			// void	update() override;
			// void	draw() override;

			void addSource(act::proc::OutputPortRef<ci::gl::Texture2dRef> outputPort);
			void removeSource(act::proc::OutputPortRef<ci::gl::Texture2dRef> outputPort);

			act::room::RoomNodeBaseRef drawMenu() override;

			virtual ci::Json toJson();
			virtual void fromJson(ci::Json json);

			inline std::map<act::UID, ci::gl::Texture2dRef> getSources() { return m_sources; }

		private:
			void refreshLists() override;

			std::map<act::UID, act::proc::InputPortRef<ci::gl::Texture2dRef>> m_ports; // outputPortUID -> InputPortRef
			std::map<act::UID, ci::gl::Texture2dRef> m_sources; // inputPortUID -> cv::UMat

		};
		using DisplayManagerRef = std::shared_ptr<DisplayManager>;
	}
}