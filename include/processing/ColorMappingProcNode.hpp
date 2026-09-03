
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


namespace act {
	namespace proc {

		class ColorMappingProcNode : public ProcNodeBase
		{
		public:
			ColorMappingProcNode();
			~ColorMappingProcNode();

			PROCNODECREATE(ColorMappingProcNode);

			void update()			override;
			void draw()				override;

			ci::Json toParams() override;
			void fromParams(ci::Json json) override;

		private:
			OutputPortRef<ci::Color>	m_colorPort;
			ci::Color m_color;

			float m_hue;
			float m_saturation;
			float m_lightness;

			bool init;

			void onHue(float number);
			void onSaturation(float number);
			void onLightness(float number);

			void sendColor();

		}; using ColorMappingProcNodeRef = std::shared_ptr<ColorMappingProcNode>;

	}
}