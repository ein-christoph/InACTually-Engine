
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

#include "RoomNodeManagerBase.hpp"

#include "DetectorBase.hpp"
#include "camera/CameraRoomNode.hpp"

#include "onnxruntime_cxx_api.h"


namespace act {
	namespace comp {


		class DepthDetector : public DetectorBase<cv::UMat> {
		public:
			DepthDetector();
			DepthDetector(room::CameraRoomNodeRef camera);
			~DepthDetector();

			static	std::shared_ptr<DepthDetector> create(room::CameraRoomNodeRef camera) { return std::make_shared<DepthDetector>(camera); };

			virtual ci::Json toJson();
			virtual void fromJson(ci::Json json);

		private:			
			static constexpr const int m_width	= 238;
			static constexpr const int m_height = 238;
			float mean[3] = { 0.485f, 0.456f, 0.406f }; 
			float std[3] = { 0.229f, 0.224f, 0.225f };

			cv::UMat									m_colorImage;
			Ort::Env									m_env;
			OrtCUDAProviderOptionsV2*					m_cudaOpts;
			Ort::Session								m_session;
			Ort::Value									m_inputTensor;
			Ort::Value									m_outputTensor;
			std::vector<std::string>					m_outputNameStrings;
			std::vector<const char*>					m_outputNames;
			std::vector<std::string>					m_inputNameStrings;
			std::vector<const char*>					m_inputNames;
			std::vector<float>							m_inputImage; // 268324 = 518*518
			std::array<float, m_width * m_height>		m_results{};

			std::array<int64_t, 4> m_inputShape{ 1, 3, m_height, m_width };
			std::array<int64_t, 3> m_outputShape{ 1, m_height, m_width };

			std::string							m_outputLayer;
			bool								m_isUsingTiny = true; 
			void initNetwork();

			bool m_detecting = true;

			std::vector<cv::Mat>				m_detection;
			cv::Size							m_blobSize;
			void detect() override;

			glm::ivec2 m_displaySize = glm::ivec2(640, 360);

		};
		using DepthDetectorRef = std::shared_ptr<DepthDetector>;
	}
}
