
/*
	InACTually
	> interactive theater for actual acts
	> this file is part of the "InACTually Engine", a MediaServer for driving all technology

	Copyright (c) 2021–2025 Lars Engeln, Fabian Töpfer
	Copyright (c) 2025 InACTually Community
	Licensed under the MIT License.
	See LICENSE file in the project root for full license information.

	This file is created and substantially modified: 2023-2025

	contributors:
	Lars Engeln - mail@lars-engeln.de
*/

#pragma once

#include "ProcNodeBase.hpp"
#include "cinder/Perlin.h"


namespace act {
	namespace proc {

		class NoiseProcNode : public ProcNodeBase
		{
		public:
			NoiseProcNode();
			~NoiseProcNode();

			PROCNODECREATE(NoiseProcNode);

			void update()			override;
			void draw()				override;

			void onMin(float min);
			void onMax(float max);

			void onSpeed(float speed);

			ci::Json toParams() override;
			void fromParams(ci::Json json) override;

		private:
			ci::Perlin						m_perlin;

			number							m_noise;

			float							m_off = false;
			float							m_min;
			float							m_max;
			float							m_speed;
			float							m_random;

			bool							m_show;

			OutputPortRef<number>			m_noiseOutPort;

		}; using NoiseProcNodeRef = std::shared_ptr<NoiseProcNode>;

	}
}
