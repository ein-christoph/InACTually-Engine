
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

#include "roompch.hpp"
#include "computing/CameraCalibrator.hpp"
#include "Logger.hpp"

act::room::CameraCalibrator::CameraCalibrator()
{

}

act::room::CameraCalibrator::~CameraCalibrator()
{

}



void act::room::CameraCalibrator::update()
{
	//m_camera->m_texture = gl::Texture::create(fromOcv(tmp), ci::gl::Texture::Format().loadTopDown());
	auto time = std::chrono::system_clock::now();
	if (m_autoTakePicutes) {
		int timeSinceLastPic = std::chrono::duration_cast<std::chrono::milliseconds>(time - m_lastPictureTaken).count();
		int secondsTilNextPic = 2 - timeSinceLastPic / 1000;
		std::string countdown = std::to_string(secondsTilNextPic);
		m_autoTakePicturesBtnText = countdown;
		if (timeSinceLastPic > 2000 && m_autoTakePicutesCnt < 50) {
			// saveCalibImage();
			m_takeImageForCalibration = true;
			m_autoTakePicutesCnt++;
			m_lastPictureTaken = std::chrono::system_clock::now();
		}
		else if (m_autoTakePicutesCnt >= 50) {
			m_autoTakePicutes = false;
			m_autoTakePicutesCnt = 0;
			m_autoTakePicturesBtnText = "Auto take images";
		}
	}
	try {
		cv::UMat image = m_camera->getCurrentImage().clone();// .getMat(cv::ACCESS_FAST);
		if (m_overlay.cols == 0)
			m_overlay = cv::UMat(image.rows, image.cols, image.type());

		if (image.empty() || image.rows == 0 || image.cols == 0)
			return;

		std::vector<cv::Point2f> corners;
		cv::Size board_sz = cv::Size(m_numCornersHor, m_numCornersVer);


		bool found = false;
		switch (m_calibPattern)
		{
		case CCP_CHESSBOARD:
			found = findChessboardCorners(image, board_sz, corners, cv::CALIB_CB_ADAPTIVE_THRESH | cv::CALIB_CB_FAST_CHECK | cv::CALIB_CB_NORMALIZE_IMAGE);
			break;
		case CCP_CHARUCOBOARD:
		{
			//ch_detector.detectBoard(view, pointbuf, markerIDs);
			//found = pointbuf.size() == (size_t)(boardSize.width - 1) * (boardSize.height - 1);
			break;
		}
		default:
			CI_LOG_E("Unknown pattern type");
		}

		/*for (auto& corner : corners) {
			corner = corner * 2;
		}*/

		if (found)
		{
			drawChessboardCorners(image, board_sz, corners, found);

			if (m_takeImageForCalibration) {
				m_takeImageForCalibration = false;

				m_camera->m_calibImages.push_back(image.clone());
				m_imagePoints.push_back(corners);

				//m_calibTextures.push_back(gl::Texture::create(fromOcv(image), ci::gl::Texture::Format().loadTopDown()));
				std::vector<cv::Point2i> rectCorners;
				int maxY = 0;
				int maxX = 0;
				int minY = 5000;
				int minX = 5000;

				for (auto corner : corners) {
					maxX = maxX > corner.x ? maxX : (int)corner.x;
					maxY = maxY > corner.y ? maxY : (int)corner.y;
					minX = minX < corner.x ? minX : (int)corner.x;
					minY = minY < corner.y ? minY : (int)corner.y;
				}

				rectCorners.push_back(cv::Point2i(minX, minY));
				rectCorners.push_back(cv::Point2i(minX, maxY));
				rectCorners.push_back(cv::Point2i(maxX, maxY));
				rectCorners.push_back(cv::Point2i(maxX, minY));

				cv::fillPoly(m_overlay, rectCorners, cv::Scalar(0, 255, 0));
				m_camera->m_calibAreas.push_back(rectCorners);
			}
		}
		else {
			//cv::cvtColor(tmp, tmp, cv::COLOR_BGR2GRAY);
			cv::putText(image, "no board found", cv::Point(50, image.rows / 2), cv::FONT_HERSHEY_DUPLEX, 2.0, CV_RGB(255, 255, 255), 5);
			//m_calibTextures.push_back(gl::Texture::create(fromOcv(tmp), ci::gl::Texture::Format().loadTopDown()));
			//drawChessboardCorners(tmp, board_sz, corners, found);
		}

		cv::addWeighted(m_overlay, 0.3, image, 0.7, 0, image);
		cv::resize(image, image, cv::Size(m_displaySize.x, m_displaySize.y));
		m_feedbackTexture = gl::Texture::create(fromOcv(image), ci::gl::Texture::Format().loadTopDown());

		/*cv::UMat overlay = cv::UMat(m_camera->m_cvSize.height, m_camera->m_cvSize.width, CV_8UC3);
		if (!m_camera->m_calibAreas.empty()) {
			for (auto area : m_camera->m_calibAreas) {
				cv::fillPoly(overlay, area, cv::Scalar(0, 255, 0));
			}
		}*/
		//cv::addWeighted(overlay, 0.3, tmp, 0.7, 0, tmp);
	}
	catch (cv::Exception exc) {
		CI_LOG_E(exc.what());
	}
}


void act::room::CameraCalibrator::draw()
{
	if (m_feedbackTexture) {
		gl::pushMatrices();
		gl::rotate(toRadians(180.0f));
		ImGui::Image(m_feedbackTexture, m_displaySize);

		gl::pushMatrices();
	}
	if (m_camera->isCalibrated()) {
		auto undist = m_camera->getUndistortedImage();
		if (!undist.empty()) {
			ImGui::SameLine();
			gl::pushMatrices();
			gl::rotate(toRadians(180.0f));
			ImGui::Image(gl::Texture::create(fromOcv(undist)), m_displaySize);
			gl::pushMatrices();
		}
	}

	ImGui::PushItemWidth(300.0);
	ImGui::InputInt("Chessboard horizontal squares", &m_numCornersHor);
	ImGui::InputInt("Chessboard vertical squares", &m_numCornersVer);
	ImGui::InputFloat("Square Size in m", &m_squareSize);
	ImGui::PopItemWidth();
	ImGui::NewLine();
	if (ImGui::Button("Take Single Image")) {
		m_takeImageForCalibration = true;
		//saveCalibImage();
	}
	if (ImGui::Button(m_autoTakePicturesBtnText.c_str())) {
		m_lastPictureTaken = std::chrono::system_clock::now();
		m_autoTakePicutes = true;
	}
	ImGui::NewLine();

	auto numberCalibImages = m_camera->m_calibImages.size();

	auto numberOfImagesText = "Number of calibrating images: " + std::to_string(numberCalibImages) + " of " + std::to_string(m_minCalibImage) + "(minimum!)";
	ImGui::Text(numberOfImagesText.c_str());

	if(numberCalibImages >= m_minCalibImage)
		if (ImGui::Button("Calibrate")) {
			m_isFinishedCalibrating = calibrate();
		}

	/*for (int i = 0; i < m_calibTextures.size(); i++) {
		gl::pushMatrices();
		if (i % 5 != 0) {
			ImGui::SameLine();
		}
		ImGui::Image(m_calibTextures[i], ivec2(240, 135));
		gl::pushMatrices();
	}*/
}

void act::room::CameraCalibrator::setCamera(CameraDeviceRef camera)
{
	if (!camera)
		return;

	// reset things
	m_calibTextures.clear();
	
	m_camera = camera;
	for (int i = 0; i < m_camera->m_calibImages.size(); i++) {
		m_calibTextures.push_back(gl::Texture::create(fromOcv(m_camera->m_calibImages[i]), ci::gl::Texture::Format().loadTopDown()));
	}
}

void act::room::CameraCalibrator::calibrateFromImages()
{
	// TODO: here from vector<UMat> to intrinsic/distcoeff
}


void act::room::CameraCalibrator::calcBoardCorners(cv::Size boardSize, float squareSize, std::vector<cv::Point3f>& corners, CameraCalibrationPattern patternType)
{
	{
		corners.resize(0);

		switch (patternType)
		{
		case CCP_CHESSBOARD:
			for (int i = 0; i < boardSize.height; i++)
				for (int j = 0; j < boardSize.width; j++)
					corners.push_back(cv::Point3f(float(j * squareSize),
						float(i * squareSize), 0));
			break;

		case CCP_CHARUCOBOARD:
			for (int i = 0; i < boardSize.height - 1; i++)
				for (int j = 0; j < boardSize.width - 1; j++)
					corners.push_back(cv::Point3f(float(j * squareSize),
						float(i * squareSize), 0));
			break;
		default:
			CI_LOG_E("Unknown pattern type");
		}
	}
}

bool act::room::CameraCalibrator::calibrate() {
	cv::Size board_sz = cv::Size(m_numCornersHor, m_numCornersVer);

	
	if (m_camera->m_calibImages.size() >= m_minCalibImage) {

		std::vector<cv::Point3f> obj;
		calcBoardCorners(board_sz, m_squareSize, obj);

		std::vector<cv::UMat>::iterator it;
		int i = 0;
		for (it = m_camera->m_calibImages.begin(); it != m_camera->m_calibImages.end(); it++, i++) {
			//bool found = findChessboardCorners(m_camera->m_calibImages[i], board_sz, m_corners, cv::CALIB_CB_ADAPTIVE_THRESH | cv::CALIB_CB_NORMALIZE_IMAGE);
			//if (found)
			{
				cv::UMat gray;
				cv::cvtColor(m_camera->m_calibImages[i], gray, cv::COLOR_BGR2GRAY);
				auto corner = &m_imagePoints[i];
				cornerSubPix(gray, *corner, cv::Size(11, 11), cv::Size(-1, -1), cv::TermCriteria(cv::TermCriteria::EPS + cv::TermCriteria::COUNT, 30, 0.0001));
				m_imagePoints[i] = *corner;
				m_objectPoints.push_back(obj);
			}

		}

		std::vector<cv::Mat> rvecs;
		std::vector<cv::Mat> tvecs;

		cv::Mat intrinsic = cv::Mat(3, 3, CV_64FC1);
		intrinsic.ptr<float>(0)[0] = 1;
		intrinsic.ptr<float>(1)[1] = 1;

		cv::Mat distCoeffs = cv::Mat(1, 5, CV_64FC1);

		cv::calibrateCamera(m_objectPoints, m_imagePoints, m_camera->m_calibImages[0].size(), intrinsic, distCoeffs, rvecs, tvecs, cv::CALIB_USE_LU);
		m_camera->setCalibration(intrinsic, distCoeffs);

		return true;
	}
	return false;
}