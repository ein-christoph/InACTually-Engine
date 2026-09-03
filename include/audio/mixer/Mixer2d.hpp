
/*
	InACTually
	> interactive theater for actual acts
	> this file is part of the "InACTually Engine", a MediaServer for driving all technology

	Copyright (c) 2021–2025 Lars Engeln, Fabian Töpfer
	Copyright (c) 2025 InACTually Community
	Licensed under the MIT License.
	See LICENSE file in the project root for full license information.

	This file is created and substantially modified: 2021-2022

	contributors:
	Lars Engeln - mail@lars-engeln.de
*/

#pragma once

#include "mixer/MixerBase.hpp"
#include "cinder/audio/GainNode.h"
#include "cinder/audio/ChannelRouterNode.h"
#include "cinder/audio/MonitorNode.h"
#include <memory>
#include <vector>
#include <optional>
#include <string>


namespace act {
	namespace aio {

		class Mixer2d : public MixerBase {
		public:
			Mixer2d();
			~Mixer2d();
			
			static std::shared_ptr<Mixer2d> create() { return std::make_shared<Mixer2d>(); };

			void update() override;

			void clear() override;

			void configure(std::vector<act::room::SpeakerRoomNodeRef> speakers
				, std::vector<act::room::SubwooferRoomNodeRef> subwoofers
				, std::vector<act::room::SoundRoomNodeRef> sounds
			) override;

			void connectSound(act::room::SoundRoomNodeRef sound
				, std::vector<act::room::SpeakerRoomNodeRef> speakers
				, std::vector<act::room::SubwooferRoomNodeRef> subwoofers
			) override;

			void updatePanningPairs();
			void updateMaxDistance();
			void updateCentroid();
			static glm::vec2 calculateIntersection( glm::vec2 p1, glm::vec2 p2
												  , glm::vec2 p3, glm::vec2 p4);
			static float calculatePanningFactor(glm::vec2 longVec, glm::vec2 shortVec);
			float calculateUnitGain(float distance);

			// Contains a shape for finding the nearest pair of speakers for a virtual
			// sound source and references to the speakers
			struct PanningPair {
				cinder::Shape2d collider;
				act::room::SpeakerRoomNodeRef speaker1;
				act::room::SpeakerRoomNodeRef speaker2;
			};


		private:
			ci::audio::ChannelRouterNodeRef											m_channelRouterNode;
			std::map<room::SoundRoomNodeRef, std::map<act::UID, ci::audio::GainNodeRef>>	m_mixMap;
			std::vector<act::room::SpeakerRoomNodeRef>								m_speakers;
			std::vector<PanningPair>												m_panningPairs;

			float																	m_pi;
			float																	m_maxDistance;
			glm::vec2																m_centroid;
			std::optional<PanningPair>												m_lastPanningPair=std::nullopt;
		};
	}


}