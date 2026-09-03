
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

#define _SILENCE_CXX17_RESULT_OF_DEPRECATION_WARNING
//#define _CRT_SECURE_NO_WARNINGS
#include "Osc.h"

#include "cinder/app/App.h"
#include "cinder/Log.h"

#include <functional>


#define USE_UDP 1
#if USE_UDP
using Receiver = ci::osc::ReceiverUdp;
using protocol = asio::ip::udp;
#else
using Receiver = ci::osc::ReceiverTcp;
using protocol = asio::ip::tcp;
#endif

namespace act {
	namespace net {

		class OSCReciever {
		public:
			OSCReciever(uint16_t port)
			{
				m_connected = false;
				m_receiver = std::make_shared<Receiver>(port);
				
#if USE_UDP
				// UDP opens the socket and "listens" accepting any message from any endpoint. The listen
				// function takes an error handler for the underlying socket. Any errors that would
				// call this function are because of problems with the socket or with the remote message.
				
#endif
			}

			static std::shared_ptr<OSCReciever> create(uint16_t localPort) { return std::make_shared<OSCReciever>(localPort); }


			void setListener(std::string address, ci::osc::ReceiverBase::ListenerFn callback) {
				m_receiver->setListener(address, callback);
			}
			void removeListener(std::string address) {
				m_receiver->removeListener(address);
			}

			void listen() {
				try{
					// Bind the receiver to the endpoint. This function may throw.
					m_receiver->bind();
				}
					catch (const ci::osc::Exception& ex) {
					CI_LOG_E("Error binding: " << ex.what() << " val: " << ex.value());
				}
				m_receiver->listen(
					[this](asio::error_code error, protocol::endpoint endpoint) -> bool {
						if (error) {
							CI_LOG_E("Error Listening: " << error.message() << " val: " << error.value() << " endpoint: " << endpoint);
							return false;
						}
						else {
							m_connected = true;
							return true;
						}
					});
			}

			bool isConnected() {
				return m_connected;
			}


		private:
			std::shared_ptr<Receiver> m_receiver;
			bool m_connected;

		};
		using OSCRecieverRef = std::shared_ptr<OSCReciever>;
	}
}