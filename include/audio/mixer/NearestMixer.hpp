
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

#include "mixer/MixerBase.hpp"
#include "cinder/audio/GainNode.h"
#include "cinder/audio/ChannelRouterNode.h"
#include "cinder/audio/MonitorNode.h"


namespace act {
	namespace aio {

		class NearestMixer : public MixerBase {
		public:
			NearestMixer();
			~NearestMixer();

			static std::shared_ptr<NearestMixer> create() { return std::make_shared<NearestMixer>(); };

			void update() override;

			void clear() override;
			void configure(std::vector<act::room::SpeakerRoomNodeRef> speakers, std::vector<act::room::SubwooferRoomNodeRef> subwoofers, std::vector<act::room::SoundRoomNodeRef> sounds) override;
			void connectSound(act::room::SoundRoomNodeRef sound, std::vector<act::room::SpeakerRoomNodeRef> speakers, std::vector<act::room::SubwooferRoomNodeRef> subwoofers) override;

		private:
			ci::audio::ChannelRouterNodeRef											m_channelRouterNode;
			std::map<room::SoundRoomNodeRef, std::map<act::UID, ci::audio::GainNodeRef>>	m_mixMap;
			std::vector<act::room::SpeakerRoomNodeRef>									m_speakers;

			float																	m_speakerDistanceNorm;
		
			void connectSpeakersToSubwoofers(std::vector<act::room::SpeakerRoomNodeRef> speakers, std::vector<act::room::SubwooferRoomNodeRef> subwoofers);
		};
	}
}