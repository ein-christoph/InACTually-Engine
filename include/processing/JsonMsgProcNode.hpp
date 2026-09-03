
/*
	InACTually
	> interactive theater for actual acts
	> this file is part of the "InACTually Engine", a MediaServer for driving all technology

	Copyright (c) 2021–2025 Lars Engeln, Fabian Töpfer
	Copyright (c) 2025 InACTually Community
	Licensed under the MIT License.
	See LICENSE file in the project root for full license information.

	This file is created and substantially modified: 2021-2024

	contributors:
	Lars Engeln - mail@lars-engeln.de
*/

#pragma once

#include "ProcNodeBase.hpp"


namespace act {
	namespace proc {

		class JsonMsgProcNode : public ProcNodeBase
		{
		public:
			JsonMsgProcNode();
			~JsonMsgProcNode();

			PROCNODECREATE(JsonMsgProcNode);

			void update()			override;
			void draw()				override;

			ci::Json toParams() override;
			void fromParams(ci::Json json) override;
			
		private:
			std::string m_msgName;
			OutputPortRef<ci::Json>	m_jsonPort;
			std::vector<PortBaseRef> m_allInputPorts;

		}; using JsonMsgProcNodeRef = std::shared_ptr<JsonMsgProcNode>;

	}
}