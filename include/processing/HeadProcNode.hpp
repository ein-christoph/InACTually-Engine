
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


namespace act {
	namespace proc {

		class HeadProcNode : public ProcNodeBase
		{
		public:
			HeadProcNode();
			~HeadProcNode();

			PROCNODECREATE(HeadProcNode);

			void setup();
			void update()			override;
			void draw()				override;

			void onBody(k4abt_skeleton_t body);

			ci::Json toParams() override;
			void fromParams(ci::Json json) override;

		private:
			
			OutputPortRef<glm::vec3>	m_directionOutput;
			OutputPortRef<glm::quat>	m_orientationOutput;

			glm::vec3				m_direction;
			glm::vec3				m_horizontal;
			glm::vec3				m_vertical;

			glm::vec3 toVec(k4a_float3_t::_xyz xyz);


		}; using HeadProcNodeRef = std::shared_ptr<HeadProcNode>;
	}
}