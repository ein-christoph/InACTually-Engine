
/*
	InACTually
	> interactive theater for actual acts
	> this file is part of the "InACTually Engine", a MediaServer for driving all technology

	Copyright (c) 2025 InACTually Community
	Licensed under the MIT License.
	See LICENSE file in the project root for full license information.

	This file is created and substantially modified: 2025

	contributors:
	Lars Engeln - mail@lars-engeln.de
*/

#pragma once

#include "ProcNodeBase.hpp"
#include "Osc.h"


namespace act {
	namespace proc {

		class OSCMsgProcNode : public ProcNodeBase
		{
		public:
			OSCMsgProcNode();
			~OSCMsgProcNode();

			PROCNODECREATE(OSCMsgProcNode);

			void update()			override;
			void draw()				override;

			ci::Json toParams() override;
			void fromParams(ci::Json json) override;
			
		private:
			std::string m_msgName;
			OutputPortRef<ci::osc::Message>	m_oscPort;
			std::vector<PortBaseRef> m_allInputPorts;

		}; using OSCMsgProcNodeRef = std::shared_ptr<OSCMsgProcNode>;

	}
}