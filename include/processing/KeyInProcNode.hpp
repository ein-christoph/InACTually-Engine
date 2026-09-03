
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

		class KeyInProcNode : public ProcNodeBase
		{
		public:
			KeyInProcNode();
			~KeyInProcNode();

			PROCNODECREATE(KeyInProcNode);

			void update()			override;
			void draw()				override;

			ci::Json toParams() override;
			void fromParams(ci::Json json) override;

		private:
			OutputPortRef<bool>	m_pressedPort;

			ImGuiKey m_key;
			ImGuiKey getCurrentPressedKey();

			bool m_isEnteringKey = false;

			bool init;

		}; using KeyInProcNodeRef = std::shared_ptr<KeyInProcNode>;

	}
}