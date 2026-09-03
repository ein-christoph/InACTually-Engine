
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

		class HandProcNode : public ProcNodeBase
		{
		public:
			HandProcNode();
			~HandProcNode();

			PROCNODECREATE(HandProcNode);

			void setup();
			void update()			override;
			void draw()				override;

			void onBody(k4abt_skeleton_t body);

			ci::Json toParams() override;
			void fromParams(ci::Json json) override;

		private:

			OutputPortRef<bool>	m_leftHandClosed;
			OutputPortRef<bool>	m_leftHandOpend;
			OutputPortRef<bool>	m_rightHandClosed;
			OutputPortRef<bool>	m_rightHandOpend;

			float				m_distanceLeft = 1.0f;
			float				m_distanceRight = 1.0f;


		}; using HandProcNodeRef = std::shared_ptr<HandProcNode>;
	}
}