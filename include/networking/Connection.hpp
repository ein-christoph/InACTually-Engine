
/*
	InACTually
	> interactive theater for actual acts
	> this file is part of the "InACTually Engine", a MediaServer for driving all technology

	Copyright (c) 2021–2025 Lars Engeln, Fabian Töpfer
	Copyright (c) 2025 InACTually Community
	Licensed under the MIT License.
	See LICENSE file in the project root for full license information.

	This file is created and substantially modified: 2023

	contributors:
	Lars Engeln - mail@lars-engeln.de
*/

#pragma once

#include <memory>
#include "cinder/Json.h"


namespace act {
	namespace net {

		class ConnectionProvider { // ConnectionProvider
		public:
			virtual void sendMsg(ci::Json json) = 0;
			virtual std::string getHostAddress() { return "not provided"; }
			virtual bool isServer() { return false; }
			virtual std::string getCurrentStatus() { return "not provided"; }
		}; using ConnectionProviderRef = std::shared_ptr<ConnectionProvider>;

		class MsgReciever {
		public:
			virtual void onMsg(ci::Json json, act::UID uid) = 0;
			virtual void onConnect(act::UID uid) = 0;
			virtual void onDisconnect(act::UID uid) = 0;
		}; using MsgRecieverRef = std::shared_ptr<MsgReciever>;

	}
}