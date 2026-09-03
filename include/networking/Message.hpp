
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

#include <memory>
#include "cinder/Json.h"
#include <functional>

#include "UniqueIDBase.hpp"
#include "Connection.hpp"


namespace act {
	namespace net {

		enum MsgType {
			MT_UNKNOWN = -1,

			MT_DESCRIPTION,

			MT_APP,
			MT_ASSET,
			MT_INTERACTION,
			MT_PROCNODE,
			MT_ROOMNODE
		};
		enum MsgMethod {
			MM_UNKNOWN	= -2,

			MM_ERROR	= -1,
			MM_WARNING	=  0,

			MM_CREATE	=  1,
			MM_REQUEST,
			MM_UPDATE,
			MM_DELETE,

			MM_CONNECT,
			MM_DISCONNECT,
			MM_SUBSCRIBE,
			MM_UNSUBSCRIBE,

			MM_UPLOAD,
			MM_REMOTEPROCEDURECALL
		};

		class Message : public UniqueIDBase {
		public:
			Message(act::UID uid = "", MsgType type = MsgType::MT_UNKNOWN, MsgMethod method = MsgMethod::MM_UNKNOWN);
			Message(ci::Json json, ConnectionProviderRef sender);

			static std::shared_ptr<Message> create() { return std::make_shared<Message>(); };
			static std::shared_ptr<Message> create(ci::Json json, ConnectionProviderRef sender) { return std::make_shared<Message>(json, sender); };

			void		setType(MsgType type) { m_type = type; }
			MsgType		getType() { return m_type; }
			void		setMethod(MsgMethod type) { m_method = type; }
			MsgMethod	getMethod() { return m_method; }
			long long	getTimestamp() { return m_timestamp; }
			void		setData(ci::Json data) { m_data = data; }
			ci::Json	getData() { return m_data; }

			bool		sendMissingError(std::string where, std::string missingItem);
			bool		sendError(std::string where, std::string what);
			ci::Json	createErrorMsgJson(std::string where, std::string what);

			bool		isValid() { return m_isValid; }

			ci::Json	toJson();
			void		fromJson(ci::Json json);

			MsgMethod	toMsgMethod(std::string method);
			std::string	fromMsgMethod(MsgMethod method);
			MsgType		toMsgType(std::string type);
			std::string	fromMsgType(MsgType type);

		private:

			bool					m_isValid	= true;
			MsgType					m_type		= MT_UNKNOWN;
			MsgMethod				m_method	= MM_UNKNOWN;
			long long				m_timestamp = 0;
			ci::Json				m_data		= ci::Json::object();
			ConnectionProviderRef	m_sender;

			bool		sendMsg(ci::Json json);
			bool		checkValidility();

		}; using MessageRef = std::shared_ptr<Message>;
	}
}