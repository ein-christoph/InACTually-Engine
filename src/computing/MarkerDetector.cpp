
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

#include "roompch.hpp"
#include "MarkerDetector.hpp"

#include <chrono>
using namespace std::chrono_literals;

act::comp::MarkerDetector::MarkerDetector() : DetectorBase("markerDetector") {
	refreshObjPoints();
}

act::comp::MarkerDetector::MarkerDetector(room::CameraRoomNodeRef camera) : DetectorBase("markerDetector", camera)
{
	refreshObjPoints();

	m_markerDictionary = cv::aruco::getPredefinedDictionary(cv::aruco::DICT_4X4_250);
	m_detectorParameters = cv::aruco::DetectorParameters();
	m_detector = cv::aruco::ArucoDetector(m_markerDictionary, m_detectorParameters);

	m_isInitialized = true;
}


act::comp::MarkerDetector::~MarkerDetector()
{
}

ci::Json act::comp::MarkerDetector::toJson() {
	auto json = ci::Json::object();

	return json;
}

void act::comp::MarkerDetector::fromJson(ci::Json json) {

}

void act::comp::MarkerDetector::refreshObjPoints()
{
	m_objPoints = cv::Mat(4, 1, CV_32FC3);
	m_objPoints.ptr<cv::Vec3f>(0)[0] = cv::Vec3f(-m_markerSize / 2.f, m_markerSize / 2.f, 0);
	m_objPoints.ptr<cv::Vec3f>(0)[1] = cv::Vec3f(m_markerSize / 2.f, m_markerSize / 2.f, 0);
	m_objPoints.ptr<cv::Vec3f>(0)[2] = cv::Vec3f(m_markerSize / 2.f, -m_markerSize / 2.f, 0);
	m_objPoints.ptr<cv::Vec3f>(0)[3] = cv::Vec3f(-m_markerSize / 2.f, -m_markerSize / 2.f, 0);
}

void  act::comp::MarkerDetector::detect() {
	
	for (auto&& it = m_newMarkerOccurence.begin(); it != m_newMarkerOccurence.end();) {
		auto m = it->second;
		if (!m->wasInLastFrame) {
			it = m_newMarkerOccurence.erase(it);
		}
		else {
			it++;
			m->wasInLastFrame = false;
		}
	}

	act::room::CameraDeviceRef camera = m_camera->getCamera();
	cv::UMat image = m_currentImage;
	if (image.empty())
		return;

	// Load the dictionary that was used to generate the markers.
	std::vector<std::vector<cv::Point2f>> markerCorners, rejectedCandidates;
	std::vector<int> markerIds;

	if(!image.empty())
		m_detector.detectMarkers(image, markerCorners, markerIds, rejectedCandidates); 
	bool originFound = false;
	int originIndex = -1;
	int i = 0;
	for (i; i < markerIds.size();) {
		if (markerIds[i] > 100) { // TODO: prevents false detections with ID above 100
			markerIds.erase(markerIds.begin() + i);
			markerCorners.erase(markerCorners.begin() + i);
			continue;
		}

		checkOccurency(markerIds[i]);

		if (!m_validMarkerIDs[markerIds[i]]) {
			markerIds.erase(markerIds.begin() + i);
			markerCorners.erase(markerCorners.begin() + i);
			continue;
		}

		
		auto corner = &markerCorners[i];
		cv::UMat gray;
		cv::cvtColor(image, gray, cv::COLOR_BGR2GRAY);
		cornerSubPix(gray, *corner, cv::Size(11, 11), cv::Size(-1, -1), cv::TermCriteria(cv::TermCriteria::EPS + cv::TermCriteria::COUNT, 30, 0.0001));
		markerCorners[i] = *corner;

		i++;
	} 

	size_t nMarkers = markerIds.size();
	std::vector<MarkerCandidate> candidates(nMarkers);

	if (markerIds.empty())
		return;
	
	// Calculate pose for each marker
	for (size_t i = 0; i < nMarkers; i++) {
		candidates[i].id = markerIds[i];
		try {
			solvePnP(m_objPoints, markerCorners.at(i), camera->getIntrinsic(), camera->getDistCoeffs(), candidates[i].rvec, candidates[i].tvec);
		}
		catch (cv::Exception exc) {
			CI_LOG_E(exc.what());
		}
	}

	cv::UMat outputImage = image.clone();
	//cv::resize(image, outputImage, cv::Size(image.cols, image.rows));
	cv::aruco::drawDetectedMarkers(outputImage, markerCorners, markerIds);
	for (unsigned int i = 0; i < markerIds.size(); i++)
		try {
			cv::drawFrameAxes(outputImage, camera->getIntrinsic(), camera->getDistCoeffs(), candidates[i].rvec, candidates[i].tvec, m_markerSize, 5);
		}
		catch (cv::Exception exc) {
			CI_LOG_E(exc.what());
		}
	m_feedbackImage = outputImage;

	m_currentCandidates = candidates;
	m_areNewCandidatesAvailable = true;
}

void act::comp::MarkerDetector::checkOccurency(int id)
{
	bool isNew = true;
	for (auto&& occurence : m_newMarkerOccurence) {
		auto&& m = occurence.second;
		if (m->id == id) {
			isNew = false;

			m->occurence++;
			m->wasInLastFrame = true;

			if (m->occurence > 10) {
				m->wasInLastFrame = false;
				m_validMarkerIDs[m->id] = true;
			}

			break;
		}
	}
	if (isNew) {
		m_newMarkerOccurence[id] = MarkerOccurence::create(id);
	}
}

glm::vec3 act::comp::MarkerDetector::transformPosition(cv::Vec3d tvec, glm::vec3 parentPosition, glm::quat parentOrientation)
{
	glm::vec3 relMarkerPos = glm::vec3(tvec[0], tvec[1], tvec[2]);
	relMarkerPos = relMarkerPos; // glm::rotateY(relMarkerPos, m_rotation.y);
	relMarkerPos.x = -relMarkerPos.x;
	relMarkerPos.y = -relMarkerPos.y;

	return parentPosition + (parentOrientation * relMarkerPos);
}

glm::mat4 act::comp::MarkerDetector::transformRotation(cv::Vec3d tvec, cv::Vec3d rvec, glm::vec3 parentPosition, glm::quat parentOrientation)
{
	glm::mat4 transform(1.0f);

	cv::Mat rotationMat, transformMat = cv::Mat::zeros(4, 4, CV_64FC1);
	rvec[2] = -rvec[2];
	cv::Rodrigues(rvec, rotationMat);

	// 3x3 to 4x4 Mat
	for (unsigned int row = 0; row < 3; ++row)
	{
		for (unsigned int col = 0; col < 3; ++col)
		{
			transformMat.at<double>(row, col) = rotationMat.at<double>(row, col);
		}
		transformMat.at<double>(row, 3) = tvec[row];
	}
	transformMat.at<double>(3, 3) = 1.0f;

	cv::Mat ocvToGl = cv::Mat::zeros(4, 4, CV_64F);
	ocvToGl.at<double>(0, 0) = 1.0f;
	ocvToGl.at<double>(1, 1) = -1.0f;
	ocvToGl.at<double>(2, 2) = -1.0f;
	ocvToGl.at<double>(3, 3) = 1.0f;
	transformMat = ocvToGl * transformMat;

	// cv::Mat to glm::mat4
	for (int i = 0; i < 4; i++)
		for (int j = 0; j < 4; j++)
			transform[i][j] = transformMat.at<double>(i, j);

	glm::mat4 parentMat = glm::mat4(parentOrientation);
	glm::mat4 offsetMat = glm::rotate(glm::pi<float>(), glm::vec3(0.0f, 1.0f, 0.0f));

	return parentMat * transform * offsetMat;
}

std::vector<cv::Vec3d> act::comp::MarkerDetector::inversePerspective(cv::Vec3d rvec, cv::Vec3d tvec) {
	rvec[0] = -rvec[0];
	tvec[0] = -tvec[0];
	
	cv::Mat invRotMat;
	cv::Rodrigues(rvec, invRotMat);

	invRotMat = invRotMat.t();
	cv::Vec3d invRvec;
	cv::Rodrigues(invRotMat, invRvec);

	cv::Vec3d invTvec = invRvec * -tvec; // ?

	std::vector<cv::Vec3d> returnVals;
	returnVals.push_back(invTvec);
	returnVals.push_back(invRvec);
	return returnVals;
}

void act::comp::MarkerDetector::setCameraInverseToMarker(cv::Vec3d rvec, cv::Vec3d tvec, glm::vec3 markerPosition, glm::quat markerOrientierung)
{
	std::vector<cv::Vec3d> invVecs = inversePerspective(rvec, tvec);
	cv::Vec3d tvecCam = invVecs[0];
	cv::Vec3d rvecCam = invVecs[1];

	m_camera->doSmoothing(true);

	glm::vec3 cameraPosition			= transformPosition(tvecCam, markerPosition, markerOrientierung);
	glm::mat4 cameraOrientierung	= transformRotation(tvecCam, rvecCam, markerPosition, markerOrientierung);
	
	m_camera->setPosition(cameraPosition);
	m_camera->setOrientation(cameraOrientierung);

	m_camera->doSmoothing(false);
}
