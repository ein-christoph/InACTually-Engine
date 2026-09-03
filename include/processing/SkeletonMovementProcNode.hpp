
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


namespace act {
	namespace proc {

		class SkeletonMovementProcNode : public ProcNodeBase
		{
		public:
			SkeletonMovementProcNode();
			~SkeletonMovementProcNode();

			PROCNODECREATE(SkeletonMovementProcNode);

			void update()			override;
			void draw()				override;

			void onBody(room::BodyRef event);

			ci::Json toParams() override;
			void fromParams(ci::Json json) override;

 
		private:
			OutputPortRef<float>	m_localMovementPort;
			OutputPortRef<float>	m_globalMovementPort;

			float calcLocalMovement(room::BodyRef body);
			float calcGlobalMovement(room::BodyRef body);

			float m_localMovement;
			float m_globalMovement;

			room::BodyRef m_oldBody;
			float   m_scaleValue;


			static bool m_registered;
		}; using SkeletonMovementProcNodeRef = std::shared_ptr<SkeletonMovementProcNode>;

	}
}