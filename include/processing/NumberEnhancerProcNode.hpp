
/*
	InACTually
	> interactive theater for actual acts
	> this file is part of the "InACTually Engine", a MediaServer for driving all technology

	Copyright (c) 2021–2025 Lars Engeln, Fabian Töpfer
	Copyright (c) 2025 InACTually Community
	Licensed under the MIT License.
	See LICENSE file in the project root for full license information.

	This file is created and substantially modified: 2021

	contributors:
	Lars Engeln - mail@lars-engeln.de
*/

#pragma once

#include "ProcNodeBase.hpp"
#include "cinder/Rand.h"


namespace act {
	namespace proc {

		class NumberEnhancerProcNode : public ProcNodeBase
		{
		public:
			NumberEnhancerProcNode();
			~NumberEnhancerProcNode();

			PROCNODECREATE(NumberEnhancerProcNode);

			void update()			override;
			void draw()				override;

			ci::Json toParams() override;
			void fromParams(ci::Json json) override;

		private:
			OutputPortRef<float>	m_numberPort;
			std::deque<float> m_history;
			std::deque<float> m_rawHistory;

			int m_historySize;
			float m_meanNumber;

			float m_currentMin;
			float m_currentMax;
			float m_mappedMin;
			float m_mappedMax;

			float m_rawInputMin;
			float m_rawInputMax;

			bool  m_doGate;
			bool  m_doMap;

			void onNumber(float number);

		}; using NumberEnhancerProcNodeRef = std::shared_ptr<NumberEnhancerProcNode>;

	}
}