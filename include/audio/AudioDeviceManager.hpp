
/*
	InACTually
	> interactive theater for actual acts
	> this file is part of the "InACTually Engine", a MediaServer for driving all technology

	Copyright (c) 2021–2025 Lars Engeln, Fabian Töpfer
	Copyright (c) 2025 InACTually Community
	Licensed under the MIT License.
	See LICENSE file in the project root for full license information.

	This file is created and substantially modified: 2021-2024

	contributors:
	Lars Engeln - mail@lars-engeln.de
*/

#pragma once

#include <memory>
#include <vector>
#include <functional>

#include "cinder/audio/audio.h"

#include "AudioDeviceListener.hpp"


namespace act {
	namespace aio {

		struct AudioDevice {
			std::string key;
			std::string name;
			std::string driver;
			int framesPerBlock;
			int inputs;
			int outputs;
		};

		class AudioDeviceManager : public AudioDeviceListener {
		public:
			AudioDeviceManager();
			~AudioDeviceManager();

			static std::shared_ptr<AudioDeviceManager> create() { return std::make_shared<AudioDeviceManager>(); };
			void init();

			void drawSettings();
			void printContextInfo();

			void refreshDeviceList();

			std::vector<AudioDevice> getInputDevices() { return m_inDevices; }
			std::vector<AudioDevice> getOutputDevices() { return m_outDevices; }

			void setupOutputDevice(std::string key);
			void setupInputDevice(std::string key);

			virtual ci::Json toJson();
			virtual void fromJson(ci::Json json);

		private:
			
			int								m_outSelection		= 0;
			int								m_inSelection		= 0;
			std::vector<std::string>		m_outKeys;
			std::vector<std::string>		m_outNames;
			std::vector<AudioDevice>		m_outDevices;
			std::vector<std::string>		m_inNames;
			std::vector<std::string>		m_inKeys;
			std::vector<AudioDevice>		m_inDevices;
			ci::audio::OutputDeviceNodeRef	m_outputDeviceNode;
			ci::audio::InputDeviceNodeRef	m_inputDeviceNode;

			std::string						m_info = "";
			void updateInfo();

			void setFormat(ci::audio::DeviceRef device); 

			void setDevice(std::string key);
			void setDevice(std::string out, std::string in);
		};
		using AudioDeviceManagerRef = std::shared_ptr<AudioDeviceManager>;
	}
}
