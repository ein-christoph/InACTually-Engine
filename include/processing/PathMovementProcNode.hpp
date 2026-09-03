
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

		class PathMovementProcNode : public ProcNodeBase
		{
		public:
			PathMovementProcNode();
			~PathMovementProcNode();

			PROCNODECREATE(PathMovementProcNode);

			void update()			override;
			void draw()				override;

			ci::Json toParams() override;
			void fromParams(ci::Json json) override;

			glm::vec3 getPosition() { return m_position; };

		private:
			OutputPortRef<glm::vec3>	m_positionPort;
			glm::vec3 m_position;

			std::vector<glm::vec3> m_points;

			void evaluate(float t);

		}; using PathMovementProcNodeRef = std::shared_ptr<PathMovementProcNode>;

	}
}