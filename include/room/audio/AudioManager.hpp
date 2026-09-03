
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

#include "cinder/audio/audio.h" 
#include "RoomNodeManagerBase.hpp"

#include <memory>
#include <vector>

#include "cinder/Capture.h"
#include "Port.hpp"

#include "AudioDeviceManager.hpp"
#include "mixer/MixerManager.hpp"

#include "audio/SoundFileRoomNode.hpp"
#include "audio/MicrophoneRoomNode.hpp"


namespace act {
	namespace room {

		class AudioManager : public RoomNodeManagerBase, public aio::AudioDeviceListener {
		public:
			AudioManager();
			~AudioManager();

			static	std::shared_ptr<AudioManager> create() { return std::make_shared<AudioManager>(); };

			void	setup() override;
			void	update() override;
			// void	draw() override;

			act::room::SoundRoomNodeRef createSound(glm::vec3 position, float radius, std::string name);
			act::room::SoundFileRoomNodeRef createSoundFile(glm::vec3 position, std::filesystem::path path, float radius, std::string name, bool noTimestretch);
			room::SpeakerRoomNodeRef	addSpeaker(int channel);
			room::SubwooferRoomNodeRef	addSubwoofer(int channel);
			room::MicrophoneRoomNodeRef	addMicrophone(int channel);

			act::room::RoomNodeBaseRef	drawMenu() override;

			virtual ci::Json toJson();
			virtual void fromJson(ci::Json json);

			virtual void onOutputDeviceChange(ci::audio::OutputDeviceNodeRef outputDevice) override;
			virtual void onInputDeviceChange(ci::audio::InputDeviceNodeRef inputDevice) override;


			std::vector<std::string> getMicrophoneNames() { return m_microphoneNames; };
			room::MicrophoneRoomNodeRef getMicrophoneByIndex(unsigned int index);
			room::MicrophoneRoomNodeRef getMicrophoneByUID(UID uid);

		private:
			aio::AudioDeviceManagerRef		m_audioDeviceMgr;
			int								m_samplerate;

			aio::MixerManagerRef			m_mixerMgr;
			ci::audio::ChannelRouterNodeRef m_inputRouter;
			ci::audio::InputDeviceNodeRef	m_inputDeviceNode;

			int											m_selectedInputChannel;
			int											m_selectedOutputChannel;
			int											m_selectedTypeIndex;
			std::vector<room::SpeakerRoomNodeRef>		m_speakers;
			std::vector<room::SubwooferRoomNodeRef>		m_subwoofers;
			std::vector<room::MicrophoneRoomNodeRef>	m_microphones;
			std::vector<std::string>					m_microphoneNames;

			std::vector<room::SoundRoomNodeRef>			m_sounds;

			void mapMicrophones();
			void refreshLists() override;
		
		};
		using AudioManagerRef = std::shared_ptr<AudioManager>;
	}
}