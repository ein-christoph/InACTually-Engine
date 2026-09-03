
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

#include "cinder/app/App.h"

#include "BodyJoint.hpp"
#include "UniqueIDBase.hpp"


namespace act {
	namespace room {
		struct Body : public UniqueIDBase {
			Body() {
				for (int i = 0; i < BJT_COUNT; i++)
					joints.push_back(BodyJoint::create((BodyJointType) i));
			};
			~Body() {}

			static std::shared_ptr<Body> create() { return std::make_shared<Body>(); };

			std::vector<BodyJointRef> joints;

			glm::vec3 getPosition() { return joints[BJT_SPINE_CHEST]->position; };

			ci::Json toJson() {
				ci::Json json = ci::Json::object();
				json["uid"] = getUID();
				ci::Json jointsJson = ci::Json::array();
				for (auto&& joint : joints) {
					jointsJson.push_back(joint->toJson());
				}
				json["joints"] = jointsJson;
				return json;
			}

			bool fromJson(ci::Json json, bool override = false) {
				if (!json.contains("uid") || !json.contains("joints"))
					return false; 
				if (override)
					setUID(json["uid"]);
				else if (getUID() != json["uid"])
					return false;

				for (auto&& joint : json["joints"]) {
					int type = -1;
					util::setValueFromJson(json, "type", type);
					joints[type]->fromJson(joint);
				}
				return json;
			}

		};	using BodyRef = std::shared_ptr<Body>;
			using BodyRefList = std::vector<BodyRef>;
	}
}