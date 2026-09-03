
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
#include "cinder/Rand.h"


namespace act {
	namespace proc {

		class SpeedProcNode : public ProcNodeBase
		{
		public:
			SpeedProcNode();
			~SpeedProcNode();

			PROCNODECREATE(SpeedProcNode);

			void update()			override;
			void draw()				override;

			ci::Json toParams() override;
			void fromParams(ci::Json json) override;

		private:
			OutputPortRef<float>	m_speedPort;
			glm::vec3				m_lastPosition;

			float					m_factor = 4.0f;

			void onPosition(glm::vec3 position);

		}; using SpeedProcNodeRef = std::shared_ptr<SpeedProcNode>;

	}
}