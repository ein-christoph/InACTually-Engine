
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

#include "roompch.hpp"
#include "dmx/DMXPro.hpp"


namespace act {
	namespace room {

		class DMXRoomNodeBase
		{
		public:
			DMXRoomNodeBase(DMXProRef dmxInterface, ci::Json description, int startAddress);
			virtual ~DMXRoomNodeBase();

			void setDMXInterface(DMXProRef dmxInterface);

			int getStartAddress() { return m_startAddress; };
			void setStartAddress(int address) { m_startAddress = address; };
			std::string getFixtureName() { return m_fixtureName; };
			int getNumberOfChannels() { return m_numberOfChannels; }


		protected:
			DMXProRef				m_dmxInterface;
			std::string				m_fixtureName;
			int						m_startAddress;
			int						m_numberOfChannels;

			bool					setValue(std::string channel, int value);

			std::map<std::string, int> m_channelMapping;
			std::map<std::string, util::MinMaxValue<int>> m_channelValues;

		}; using DMXRoomNodeBaseRef = std::shared_ptr<DMXRoomNodeBase>;
		
	}
}