
/*
	InACTually
	> interactive theater for actual acts
	> this file is part of the "InACTually Engine", a MediaServer for driving all technology

	Copyright (c) 2021–2025 Lars Engeln, Fabian Töpfer
	Copyright (c) 2025 InACTually Community
	Licensed under the MIT License.
	See LICENSE file in the project root for full license information.

	This file is created and substantially modified: 2023-2024

	contributors:
	Lars Engeln - mail@lars-engeln.de
*/

#pragma once

#include "camera/CameraDevice.hpp"


namespace act {
	namespace room {

		enum CameraCalibrationPattern { CCP_UNKOWN = -1, CCP_CHESSBOARD = 0, CCP_CHARUCOBOARD };

		class CameraCalibrator
		{
		public:
			CameraCalibrator();
			virtual ~CameraCalibrator();

			static std::shared_ptr<CameraCalibrator> create() { return std::make_shared<CameraCalibrator>(); };
			
			virtual void update();
			virtual void draw();

			bool isFinishedCalibrating() {
				return m_isFinishedCalibrating;
			};

			void setCamera(CameraDeviceRef camera);
			void calibrateFromImages();


		private:

			CameraDeviceRef m_camera;

			bool m_isFinishedCalibrating = false;

			CameraCalibrationPattern m_calibPattern = CCP_CHESSBOARD;
			void calcBoardCorners(cv::Size boardSize, float squareSize, std::vector<cv::Point3f>& corners, CameraCalibrationPattern patternType = CCP_CHESSBOARD);
			
			bool calibrate();

			

			int				m_minCalibImage = 5;
			std::string		m_autoTakePicturesBtnText = "Auto take images";
			bool			m_autoTakePicutes = false;
			int				m_autoTakePicutesCnt = 0;
			std::chrono::system_clock::time_point	m_lastPictureTaken;

			int				m_numCornersHor = 4;
			int				m_numCornersVer = 7;
			float			m_squareSize = 0.035f;
			cv::Mat			m_cvImage;
			cv::Mat			m_cvGrayImage;
			bool			m_takeImageForCalibration = false;

			std::vector<std::vector<cv::Point3f>>	m_objectPoints;
			std::vector<std::vector<cv::Point2f>>	m_imagePoints;

			glm::ivec2								m_displaySize = glm::ivec2(640, 360);

			std::vector<ci::gl::TextureRef>			m_calibTextures;
			cv::UMat								m_overlay;
			ci::gl::TextureRef						m_feedbackTexture;

		}; using CameraCalibratorRef = std::shared_ptr<CameraCalibrator>;
		
	}
}