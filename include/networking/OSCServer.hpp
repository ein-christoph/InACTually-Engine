
/*
	InACTually
	> interactive theater for actual acts
	> this file is part of the "InACTually Engine", a MediaServer for driving all technology

	Copyright (c) 2021–2025 Lars Engeln, Fabian Töpfer
	Copyright (c) 2025 InACTually Community
	Licensed under the MIT License.
	See LICENSE file in the project root for full license information.

	This file is created and substantially modified: 2021-2023

	contributors:
	Lars Engeln - mail@lars-engeln.de
*/

#pragma once

#include "Osc.h"


using Sender = ci::osc::SenderUdp;
#define _SILENCE_CXX17_RESULT_OF_DEPRECATION_WARNING


namespace act {
	namespace net {

		class OSCServer
		{
		public:
			OSCServer(uint16_t localPort, asio::ip::address address = asio::ip::address_v4::broadcast());
			~OSCServer();

			static std::shared_ptr<OSCServer> create(uint16_t localPort, asio::ip::address address = asio::ip::address_v4::broadcast()) { return std::make_shared<OSCServer>(localPort, address); }

			inline bool isRunning() { return m_isConnected; }

			void sendMsg(ci::osc::Message msg);

		private:
			ci::osc::UdpSocketRef	m_socket;
			Sender	m_sender;
			bool	m_isConnected;
			void	onSendError(asio::error_code error);
		}; 
		using OSCServerRef = std::shared_ptr<OSCServer>;
	}
}