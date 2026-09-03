
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

#include "ProcNodeBase.hpp"

#include <chrono>

using namespace std::chrono;


namespace act {
	namespace proc {

		class BoneVectorProcNode : public ProcNodeBase
		{
		public:
			BoneVectorProcNode();
			~BoneVectorProcNode();

			PROCNODECREATE(BoneVectorProcNode);

			void setup();
			void update()			override;
			void draw()				override;

			void onSkeleton(room::BodyRef event);

			ci::Json toParams() override;
			void fromParams(ci::Json json) override;

		private:
			room::BodyRef Skeleton;

			glm::vec3 m_currentVec;
			bool m_doNormalize;

			const char* m_jointSelection[14] = {
				"01 - PELVIS",
				"02 - SPINE_CHEST",
				"03 - NECK",
				"05 - SHOULDER_LEFT",
				"06 - ELBOW_LEFT",
				"07 - HAND_LEFT",
				"08 - SHOULDER_RIGHT",
				"09 - ELBOW_RIGHT",
				"10 - HAND_RIGHT",
				"11 - KNEE_LEFT",
				"12 - FOOT_LEFT",
				"13 - KNEE_RIGHT",
				"14 - FOOT_RIGHT",
				"15 - HEAD"
			};

			int m_fromJoint;
			int m_toJoint;

			OutputPortRef<glm::vec3>	m_positionOutPort;

			static bool	m_registered;

		}; using BoneVectorNodeRef = std::shared_ptr<BoneVectorProcNode>;
	}
}