
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

#include "mixer/MixerBase.hpp"
#include "cinder/audio/GainNode.h"
#include "cinder/audio/ChannelRouterNode.h"
#include "cinder/audio/MonitorNode.h"
#include <vector>
#include <optional>


namespace act {
	namespace aio {

		class Mixer3d :public MixerBase {
		public:
			Mixer3d();
			~Mixer3d();

			static std::shared_ptr<Mixer3d> create() { return std::make_shared<Mixer3d>(); }

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
			
			// calculate intersection between line spanned by p0 and p1 and plane a float4 unit hessian normal form (fourth component -d)
			static bool calculate_intersection_line_plane(glm::vec3 p0, glm::vec3 p1, ci::vec4 plane, glm::vec3& intersection, float epsilon = 1e-6f);

			void updateColliders();
			void updateMaxDistance();
			static glm::vec2 calculateIntersection(glm::vec2 p1, glm::vec2 p2
				, glm::vec2 p3, glm::vec2 p4);

			float calculateUnitGain(float distance);

			// contains the speakers belonging to a merged face and their centroid
			struct MergedFace {
				glm::vec3 planeCentroid;
				std::vector<act::room::SpeakerRoomNodeRef> speakers;
			};
			// used for finding the right speakers for panning
			struct Collider {
				std::vector<act::room::SpeakerRoomNodeRef> speakers;
				std::optional<MergedFace> mergedFace;
				glm::vec3 normal;
			};

			Collider findCollider(glm::vec3 soundPos);

			glm::vec3 calculatePlaneIntersection(const glm::vec3& normal, const glm::vec3& planePoint, const glm::vec3& soundPos);


		private:
			ci::audio::ChannelRouterNodeRef													m_channelRouterNode;
			std::map<room::SoundRoomNodeRef, std::map<act::UID, ci::audio::GainNodeRef>>	m_mixMap;
			std::vector<act::room::SpeakerRoomNodeRef>										m_speakers;
			std::vector<Collider>															m_colliders;

			float																			m_maxDistance;
			float																			m_minDistance;
			glm::vec3																		m_centroid;
			std::optional<Collider>															m_lastCollider = std::nullopt;
		};
	}
}
