
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

		class MarkerDetectionProcNode : public ProcNodeBase
		{
		public:
			MarkerDetectionProcNode();
			~MarkerDetectionProcNode();

			PROCNODECREATE(MarkerDetectionProcNode);

			void update()			override;
			void draw()				override;

			void onMat(cv::UMat event);

			ci::Json toParams() override;
			void fromParams(ci::Json json) override;

		private:
			ci::gl::Texture2dRef	m_texture;
			float	m_displayScale;

			ImageOutputPortRef		m_markerPort;
			ImageOutputPortRef		m_tinyMarkerPort;
			OutputPortRef<float>			m_markerIDPort;

			int		m_min, m_max;
			float	m_approximation;
			float	m_distanceThreshold;

			bool	m_show;
			
			float	m_resizeScale;

			bool isBlack(cv::Mat mat, cv::Point pt);
			
		}; using MarkerDetectionProcNodeRef = std::shared_ptr<MarkerDetectionProcNode>;

	}
}