
/*
	InACTually
	> interactive theater for actual acts
	> this file is part of the "InACTually Engine", a MediaServer for driving all technology

	Copyright (c) 2021–2025 Lars Engeln, Fabian Töpfer
	Copyright (c) 2025 InACTually Community
	Licensed under the MIT License.
	See LICENSE file in the project root for full license information.

	This file is created and substantially modified: 2024

	contributors:
	Lars Engeln - mail@lars-engeln.de
*/

#pragma once

#include "ModuleBase.hpp"
#include "ModuleRegistry.hpp"
#include "server_wss.hpp"
#include "Connection.hpp"


namespace act {
	namespace net {

		using WssServer = SimpleWeb::SocketServer<SimpleWeb::WSS>;
		using WssServerRef = std::shared_ptr<WssServer>;

		class WebUISecureServer : public ConnectionProvider, public UniqueIDBase
		{
		public:
			WebUISecureServer(MsgRecieverRef reciever);
			~WebUISecureServer();

			static std::shared_ptr<WebUISecureServer> create(MsgRecieverRef reciever) { return std::make_shared<WebUISecureServer>(reciever); }

			void				update();
			void				draw();


			ci::Json			toJson();
			void				fromJson(ci::Json json);

			void				sendMsg(ci::Json msg);

			bool				isConnected() { return m_isConnected; }

		private:
			MsgRecieverRef		m_reciever;

			WssServerRef		m_server;
			std::thread			m_serverThread;
			unsigned int		m_port;
			bool				m_isConnected;

			std::string			m_text;
			
			void				recieveJson(ci::Json json);


		}; using WebUISecureServerRef = std::shared_ptr<WebUISecureServer>;

	}
}