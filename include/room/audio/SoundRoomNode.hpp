
/*
	InACTually
	> interactive theater for actual acts
	> this file is part of the "InACTually Engine", a MediaServer for driving all technology

	Copyright (c) 2021–2025 Lars Engeln, Fabian Töpfer
	Copyright (c) 2025 InACTually Community
	Licensed under the MIT License.
	See LICENSE file in the project root for full license information.

	This file is created and substantially modified: 2022-2024

	contributors:
	Lars Engeln - mail@lars-engeln.de
*/

#pragma once

#include "audio/AudioRoomNodeBase.hpp"
#include "cinder/audio/GenNode.h"
#include "cinder/audio/audio.h"
#include <memory>


namespace act {
	namespace room {

		class SoundRoomNode : public AudioRoomNodeBase {
		public:
			SoundRoomNode(glm::vec3 position, float radius, std::string name = "");
			~SoundRoomNode();

			static std::shared_ptr<SoundRoomNode> create(glm::vec3 position, float radius, std::string name = "") { return std::make_shared<SoundRoomNode>(position, radius, name); };

			std::string		getName() { return m_name; };

			virtual void	setup()		override;
			virtual void	update()	override;
			virtual void	draw()		override;

			void			disconnectExternals() override;

		protected:
			std::string m_name;

			virtual void drawSpecificSettings() override;
		};
		using SoundRoomNodeRef = std::shared_ptr<SoundRoomNode>;
	}
}