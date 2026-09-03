
/*
	InACTually
	> interactive theater for actual acts
	> this file is part of the "InACTually Engine", a MediaServer for driving all technology

	Copyright (c) 2021–2025 Lars Engeln, Fabian Töpfer
	Copyright (c) 2025 InACTually Community
	Licensed under the MIT License.
	See LICENSE file in the project root for full license information.

	This file is created and substantially modified: 2022

	contributors:
	Lars Engeln - mail@lars-engeln.de
*/

#pragma once

#include "ProcNodeBase.hpp"

#include <chrono>

using namespace std::chrono;

 
namespace act {
	namespace proc {

		class SkeletonFilterProcNode : public ProcNodeBase
		{
		public:
			SkeletonFilterProcNode();
			~SkeletonFilterProcNode();

			PROCNODECREATE(SkeletonFilterProcNode);

			void setup();
			void update()			override;
			void draw()				override;

			void onSkeleton(std::tuple<uint32_t, k4abt_skeleton_t> event);

			ci::Json toParams() override;
			void fromParams(ci::Json json) override;

		private:
			std::tuple<uint32_t, k4abt_skeleton_t> Skeleton;

			glm::vec3 m_currentJointPosition;

			const char* m_jointSelection[32] = {
				"01 - PELVIS",
				"02 - SPINE_NAVAL",
				"03 - SPINE_CHEST",
				"04 - NECK",
				"05 - CLAVICLE_LEFT",
				"06 - SHOULDER_LEFT",
				"07 - ELBOW_LEFT",
				"08 - WRIST_LEFT",
				"09 - HAND_LEFT",
				"10 - HANDTIP_LEFT",
				"11 - THUMB_LEFT",
				"12 - CLAVICLE_RIGHT",
				"13 - SHOULDER_RIGHT",
				"14 - ELBOW_RIGHT",
				"15 - WRIST_RIGHT",
				"16 - HAND_RIGHT",
				"17 - HANDTIP_RIGHT",
				"18 - THUMB_RIGHT",
				"19 - HIP_LEFT",
				"20 - KNEE_LEFT",
				"21 - ANKLE_LEFT",
				"22 - FOOT_LEFT",
				"23 - HIP_RIGHT",
				"24 - KNEE_RIGHT",
				"25 - ANKLE_RIGHT",
				"26 - FOOT_RIGHT",
				"27 - HEAD",
				"28 - NOSE",
				"29 - EYE_LEFT",
				"30 - EAR_LEFT",
				"31 - EYE_RIGHT",
				"32 - EAR_RIGHT"
			};

			int m_currentJoint;

			OutputPortRef<glm::vec3>	m_positionOutPort;

			static bool	m_registered;

		}; using SkeletonFilterProcNodeRef = std::shared_ptr<SkeletonFilterProcNode>;
	}
}