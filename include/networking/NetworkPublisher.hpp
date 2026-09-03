
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

#include "roompch.hpp"
#include "UniqueIDBase.hpp"


namespace act {
	namespace net {

		enum PublishType {
			PT_UNKNOWN,

			PT_ROOMNODE_CREATE,
			PT_ROOMNODE_UPDATE,
			PT_ROOMNODE_DELETE
		};
		
		class NetworkPublisher
		{
		public:
			NetworkPublisher() {};
			virtual ~NetworkPublisher() {};

			void publishChanges(act::UID nodeUID, std::string key, ci::Json value, PublishType type = PT_ROOMNODE_UPDATE, act::UID replyUID = "") {
				ci::Json data = ci::Json::object();
				if(replyUID.empty())
					replyUID = UniqueIDBase().getUID();

				if (key == "data") {
					data = value;
				}
				else if (key == "params") {
					data["params"] = value;
				}
				else {
					data[key] = value;
				}
				data["uid"] = nodeUID;
				publishChanges(replyUID, data, type);
			};
			virtual void publishChanges(act::UID replyUID, ci::Json data, PublishType type = PT_ROOMNODE_UPDATE) = 0;

		protected:
			
		};
		using NetworkPublisherRef = std::shared_ptr<NetworkPublisher>;
	}
}