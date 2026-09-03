
/*
	InACTually
	> interactive theater for actual acts
	> this file is part of the "InACTually Engine", a MediaServer for driving all technology

	Copyright (c) 2021–2025 Lars Engeln, Fabian Töpfer
	Copyright (c) 2025 InACTually Community
	Licensed under the MIT License.
	See LICENSE file in the project root for full license information.

	This file is created and substantially modified: 2022

	contributors:
	Lars Engeln - mail@lars-engeln.de
*/

#pragma once

#include "audio/SpeakerRoomNode.hpp"


namespace act {
	namespace room {

		class SubwooferRoomNode : public SpeakerRoomNode {
		public:
			SubwooferRoomNode(int channel, glm::vec3 position, float radius);
			~SubwooferRoomNode();

			static std::shared_ptr<SubwooferRoomNode> create(int channel, glm::vec3 position, float radius) { return std::make_shared<SubwooferRoomNode>(channel, position, radius); };

			virtual void		draw()		override;

		private:

			void drawSpecificSettings() override;
		};
		using SubwooferRoomNodeRef = std::shared_ptr<SubwooferRoomNode>;
	}
}