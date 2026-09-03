
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
#include <memory>
#include <vector>

#include "RoomNodeBase.hpp"
#include "cinder/audio/Context.h"
#include "cinder/audio/GainNode.h"
#include "cinder/Timeline.h"


namespace act {
	namespace room {

		class AudioRoomNodeBase : public RoomNodeBase
		{
		public:
			AudioRoomNodeBase(std::string name, glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 rotation = glm::vec3(0.0f, 0.0f, 0.0f), float radius = 0.0f);
			virtual ~AudioRoomNodeBase();

			virtual void				setVolume(float volume, float rampDuration = 0.1f);
			virtual void				rampVolume(float volume, float rampDuration = 0.1f);
			virtual float				getVolume() { return m_volume.value(); };
			virtual ci::audio::NodeRef	getIn() { return m_gain; }
			virtual ci::audio::NodeRef	getOut() { return m_gain; }
			virtual void				disconnectExternals() = 0;

		protected:
			ci::audio::GainNodeRef	m_gain;
			ci::Anim<float>			m_volume;

		private:			

		}; using AudioRoomNodeBaseRef = std::shared_ptr<AudioRoomNodeBase>;

	}
}