
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

#include <memory>
#include <vector>

#include "cinder/audio/audio.h"
#include "audio/SpeakerRoomNode.hpp"
#include "audio/SubwooferRoomNode.hpp"
#include "audio/SoundRoomNode.hpp"


namespace act {
	namespace aio {

		class MixerBase {
		public:
			MixerBase();
			~MixerBase();

			virtual void update() = 0;

			virtual void clear() = 0;
			virtual void connectSound(act::room::SoundRoomNodeRef sound, std::vector<act::room::SpeakerRoomNodeRef> speakers, std::vector<act::room::SubwooferRoomNodeRef> subwoofers) = 0;
			virtual void configure(std::vector<act::room::SpeakerRoomNodeRef> speakers, std::vector<act::room::SubwooferRoomNodeRef> subwoofers, std::vector<act::room::SoundRoomNodeRef> sounds) = 0;

		protected:
			
		};
		using MixerRef = std::shared_ptr<MixerBase>;
	}
}