
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

#include "opencv2/core.hpp"


namespace act {
	namespace proc {

		class ImageEnhancerProcNode : public ProcNodeBase
		{
		public:
			ImageEnhancerProcNode();
			~ImageEnhancerProcNode();

			PROCNODECREATE(ImageEnhancerProcNode);

			void update()			override;
			void draw()				override;

			void onMat(cv::UMat event);

			ci::Json toParams() override;
			void fromParams(ci::Json json) override;

		private:
			ci::gl::Texture2dRef	m_texture;

			ImageOutputPortRef	m_imagePort;

			bool m_toGrayScale;
			bool m_equalize;
			bool m_adaptiveLuminance;
			bool m_horizontalFlip;
			bool m_verticalFlip;
			bool m_tiltLeft;
			bool m_tiltRight;
			bool m_colorInvert;
			float m_cropT;
			float m_cropB;
			float m_cropL;
			float m_cropR;
			float m_resize;
			bool m_blur;

			int m_blurKernel;

			bool m_show;

			cv::Ptr<cv::CLAHE> m_clahe;
			int m_claheClip;

		}; using ImageEnhancerProcNodeRef = std::shared_ptr<ImageEnhancerProcNode>;

	}
}