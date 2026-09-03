
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

#include <glm/glm.hpp>
#include <map>


namespace act {
	namespace room {
		enum BodyJointType {
			BJT_UNKNOWN = -1,
			BJT_PELVIS = 0,
			BJT_SPINE_CHEST,
			BJT_NECK,
			BJT_SHOULDER_LEFT,
			BJT_ELBOW_LEFT,
			BJT_HAND_LEFT,
			BJT_SHOULDER_RIGHT,
			BJT_ELBOW_RIGHT,
			BJT_HAND_RIGHT,
			BJT_KNEE_LEFT,
			BJT_FOOT_LEFT,
			BJT_KNEE_RIGHT,
			BJT_FOOT_RIGHT,
			BJT_HEAD,
			BJT_COUNT
		};

		const int bodyJointParentLookUp[14] = {
			BJT_PELVIS,
			BJT_PELVIS,
			BJT_SPINE_CHEST,
			BJT_NECK,
			BJT_SHOULDER_LEFT,
			BJT_ELBOW_LEFT,
			BJT_NECK,
			BJT_SHOULDER_RIGHT,
			BJT_ELBOW_RIGHT,
			BJT_PELVIS,
			BJT_KNEE_LEFT,
			BJT_PELVIS,
			BJT_KNEE_RIGHT,
			BJT_NECK
		};

		enum BodyJointConfidence {
			BJC_NONE	= -1,     /**< Joint is out of range */
			BJC_UNKNOWN = 0,      /**< Joint is out of range */
			BJC_LOW		= 1,      /**< Joint is not observed, predicted joint pose */
			BJC_HIGH	= 2,      /**< High confidence in observed joint pose */
		};

		
		struct BodyJointInfo {
			BodyJointInfo(BodyJointType type, glm::vec3 TPoseDirection) {
				this->nextJoint = type;
				this->TPoseDirection = TPoseDirection;
			}
			~BodyJointInfo() {};

			static std::shared_ptr<BodyJointInfo> create(BodyJointType type = BJT_UNKNOWN, glm::vec3 position = glm::vec3(0, 0, 0)) { return std::make_shared<BodyJointInfo>(type, position); };


			BodyJointType nextJoint;
			glm::vec3 TPoseDirection;
		}; using BodyJointInfoRef = std::shared_ptr<BodyJointInfo>;

	 

 
		static std::map<BodyJointType, BodyJointInfoRef> bodyMap = std::map<BodyJointType, BodyJointInfoRef>
		{
			{BJT_PELVIS				,BodyJointInfo::create(BJT_SPINE_CHEST		,glm::vec3(0.0f, 1.0f, 0.0f))},
			{BJT_SPINE_CHEST		,BodyJointInfo::create(BJT_NECK				,glm::vec3(0.0f, 1.0f, 0.0f))},
			{BJT_NECK				,BodyJointInfo::create(BJT_HEAD				,glm::vec3(0.0f, 1.0f, 0.0f))},

			{BJT_SHOULDER_LEFT		,BodyJointInfo::create(BJT_ELBOW_LEFT		,glm::vec3(-1.0f, 0.0f, 0.0f))},
			{BJT_ELBOW_LEFT			,BodyJointInfo::create(BJT_HAND_LEFT		,glm::vec3(-1.0f, 0.0f, 0.0f))},
			{BJT_KNEE_LEFT			,BodyJointInfo::create(BJT_FOOT_LEFT		,glm::vec3(0.0f, -1.0f, 0.0f))},

			{BJT_SHOULDER_RIGHT		,BodyJointInfo::create(BJT_ELBOW_RIGHT		,glm::vec3(1.0f, 0.0f, 0.0f))},
			{BJT_ELBOW_RIGHT		,BodyJointInfo::create(BJT_HAND_RIGHT		,glm::vec3(1.0f, 0.0f, 0.0f))},
			{BJT_KNEE_RIGHT			,BodyJointInfo::create(BJT_FOOT_RIGHT		,glm::vec3(0.0f, -1.0f, 0.0f))},
		};

		struct BodyJoint {
			BodyJoint(BodyJointType type = BJT_UNKNOWN, glm::vec3 position = glm::vec3(0, 0, 0), glm::quat orientation = glm::quat(0, 0, 0, 1), BodyJointConfidence confidenceLevel = BJC_UNKNOWN){
				this->position = position;
				this->orientation = orientation;
				this->confidenceLevel = confidenceLevel;
				this->type = type;
			};
			~BodyJoint() {};

			static std::shared_ptr<BodyJoint> create(BodyJointType type = BJT_UNKNOWN, glm::vec3 position = glm::vec3(0, 0, 0), glm::quat orientation = glm::quat(0, 0, 0, 1), BodyJointConfidence confidenceLevel = BJC_UNKNOWN) { return std::make_shared<BodyJoint>(type, position, orientation, confidenceLevel); };

			BodyJointType type;
			glm::vec3 position;
			glm::quat orientation;
			BodyJointConfidence confidenceLevel;
			
			ci::Json toJson() {
				ci::Json json = ci::Json::object();
				json["type"] = type;
				json["position"] = util::valueToJson(position, 2);
				json["orientation"] = util::valueToJson(orientation, 3);
				json["confidenceLevel"] = confidenceLevel;
				return json;
			}

			void fromJson(ci::Json json) {
				if (json.contains("type") && type == BJT_UNKNOWN) {
					type = json["type"]; // if update, type should not have been changed - only change if it was unknown before
				}
				util::setValueFromJson(json, "position", position);
				util::setValueFromJson(json, "orientation", orientation);
				if (json.contains("confidenceLevel"))
					confidenceLevel = json["confidenceLevel"];
			}

		}; using BodyJointRef = std::shared_ptr<BodyJoint>;
	}
}