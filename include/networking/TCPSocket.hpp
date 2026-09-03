
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

#include "ModuleBase.hpp"
#include "ModuleRegistry.hpp"

#include "zmq.hpp"
#include <zmq_addon.hpp>

#include "Connection.hpp"


namespace act {
	namespace net {

		class TCPSocket : public ConnectionProvider, public UniqueIDBase
		{
		public:
			TCPSocket(MsgRecieverRef reciever);
			~TCPSocket();

			static std::shared_ptr<TCPSocket> create(MsgRecieverRef reciever) { return std::make_shared<TCPSocket>(reciever); }

			
			void				draw();


			ci::Json		toJson();
			void				fromJson(ci::Json json);

			virtual void		sendMsg(ci::Json msg) override;
			virtual std::string getHostAddress() override;
			virtual bool		isServer() override { return m_isServer; };
			virtual std::string getCurrentStatus() override;

		private:
			MsgRecieverRef						m_reciever;

			std::shared_ptr<zmq::context_t>		m_ctx;
			std::shared_ptr<zmq::socket_t>		m_socket;
			unsigned int						m_port;

			bool								m_isServer;

			std::string							m_text;
			std::thread							m_recvThread;

			bool				establishServer();
			void				connectAsClient();
			void				listen();
			
			void				recieveJson(ci::Json json);


		}; using TCPSocketRef = std::shared_ptr<TCPSocket>;

	}
}