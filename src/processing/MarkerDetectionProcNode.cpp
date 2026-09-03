
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

#include "procpch.hpp"
#include "MarkerDetectionProcNode.hpp"

act::proc::MarkerDetectionProcNode::MarkerDetectionProcNode() : ProcNodeBase("MarkerDetection") {

	m_displayScale = 0.8f;
	m_resizeScale = 0.5f;

	m_show = false;

	m_min = 4;
	m_max = 4;
	m_approximation = 20.0f;
	m_distanceThreshold = 4.0f;
	
	auto image = createImageInput("image", [&](cv::UMat mat) { this->onMat(mat); });

	m_markerPort = createImageOutput("marker image");
	m_tinyMarkerPort = createImageOutput("thresholded image");
	m_markerIDPort = createNumberOutput("marker id");
}

act::proc::MarkerDetectionProcNode::~MarkerDetectionProcNode() {
}

void act::proc::MarkerDetectionProcNode::update() {
}

void act::proc::MarkerDetectionProcNode::draw() {
	beginNodeDraw();

	ImGui::Checkbox("show", &m_show);

	preventDrag(true);
	if (m_show && m_texture) {
		ci::gl::pushMatrices();
		//ci::gl::rotate(ci::toRadians(180.0f));
		glm::vec2 texSize = ci::Rectf(m_texture->getBounds()).getCenteredFit(ci::Rectf(glm::ivec2(0, 0), m_drawSize), true).getSize();
		ImGui::Image(m_texture, texSize, glm::vec2(1, 1), glm::vec2(0, 0));

		ci::gl::pushMatrices();
	}

	bool isSlider = false;
	
	isSlider = ImGui::SliderFloat("resize", &m_resizeScale, 0.1f, 1.2f);

	if (ImGui::SliderFloat("approximation", &m_approximation, 0.001, 75.000))
		isSlider = true;

	if (ImGui::SliderFloat("distanceTheshold", &m_distanceThreshold, 0.001, 15.000))
		isSlider = true;
	
	if (ImGui::SliderInt("min", &m_min, 1, m_max))
		isSlider = true;
	if (ImGui::SliderInt("max", &m_max, m_min, 500))
		isSlider = true;
	
	preventDrag(isSlider);
	
	endNodeDraw();
}

void act::proc::MarkerDetectionProcNode::onMat(cv::UMat event) {
	cv::UMat marker;
	cv::UMat canny;
	
	cv::resize(event, marker, cv::Size(event.cols * m_resizeScale, event.rows * m_resizeScale));
	
	
	cv::cvtColor(marker, marker, cv::COLOR_BGR2GRAY);
	cv::blur(marker, canny, cv::Size(3, 3));

	
	std::vector<std::vector<cv::Point>>  contours;
	std::vector<std::vector<cv::Point>>  candidates;
	std::vector<cv::Vec4i>				 hierarchy;

	int     thresh = 100;
	int     max_thresh = 255;

	/// Detect edges using canny
	Canny(canny, canny, thresh, thresh * 2, 3);
	/// Find contours
	findContours(canny, contours, hierarchy, cv::RETR_LIST, cv::CHAIN_APPROX_SIMPLE, cv::Point(0, 0));

	cv::cvtColor(canny, canny, cv::COLOR_GRAY2BGR);
	
	/// Draw contours
	///marker = cv::UMat::zeros(canny_output.size(), CV_8UC3);
	for (int i = 0; i < contours.size(); i++)
	{
		auto&& contour = contours[i];
		cv::approxPolyDP(contour, contour, m_approximation, true);

		if (contour.size() >= m_min && contour.size() <= m_max) {
			auto m = cv::moments(contour);
			auto center = cv::Point2f(m.m10 / m.m00, m.m01 / m.m00);

			if (cv::pointPolygonTest(contour, center, false) > 0.0f) {
				cv::RotatedRect boundingBox = cv::minAreaRect(contour);
				if (boundingBox.boundingRect().area() > 10 && m_distanceThreshold > cv::norm(boundingBox.center - center)) {
					candidates.push_back(contour);
					drawContours(canny, contours, i, ci::toOcv(util::Design::primaryColor()), 2, 8, hierarchy, 0, cv::Point());
				}
			}
		}
	}

	std::vector<std::pair<int,cv::Point2f>> validmarker;

	for (int i = 0; i < candidates.size(); i++) {
		auto moments = cv::moments(candidates[i]);
		auto center = cv::Point2f(moments.m10 / moments.m00, moments.m01 / moments.m00);

		bool discard = false;
		for(auto&& wm : validmarker) {
			CI_LOG_D(cv::norm(wm.second - center));
			if(cv::norm(wm.second - center) <= 1.0f) {
				discard = true;
				break;
			}
		}
		if (discard)
			continue;

		
		cv::Point2f srcPoints[4];
		srcPoints[0] = candidates[i][0];
		srcPoints[1] = candidates[i][1];
		srcPoints[2] = candidates[i][2];
		srcPoints[3] = candidates[i][3];
		cv::Point2f dstPoints[4];
		dstPoints[0] = cv::Point(0, 500);
		dstPoints[1] = cv::Point(0, 0);
		dstPoints[2] = cv::Point(500, 0);
		dstPoints[3] = cv::Point(500, 500);
		cv::Mat warpMatrix = cv::getPerspectiveTransform(srcPoints, dstPoints);
		// determine bounding rectangle, center not relevant
		cv::UMat tmp;
		//cv::UMat markerROI = marker(roi & cv::Rect(0, 0, marker.cols, marker.rows));
		warpPerspective(marker, tmp, warpMatrix, cv::Size(500,500), cv::INTER_LINEAR, cv::BORDER_CONSTANT);
	
		
		m_markerPort->send(tmp);

		
		cv::resize(tmp, marker, cv::Size(6, 6), 0, 0, cv::INTER_LINEAR);
		cv::Scalar mean = cv::mean(marker);
		cv::threshold(marker, marker, mean[0], 255, cv::THRESH_BINARY);

		cv::Mat m = marker.getMat(cv::ACCESS_FAST);
		cv::Mat validCheck = m.clone();
		validCheck(cv::Rect(1, 1, 4, 4)) = cv::Mat::zeros(4, 4, m.type());
		int cornerCheck = m.at<uchar>(cv::Point(1, 1)) + m.at<uchar>(cv::Point(1, 4)) + m.at<uchar>(cv::Point(4, 4)) + m.at<uchar>(cv::Point(4, 1));
		if(cv::mean(validCheck)[0] == 0 && cornerCheck == 765) {

			drawContours(canny, candidates, i, ci::toOcv(util::Design::additionalColor()), 2, 8, hierarchy, 0, cv::Point());
			
			if (isBlack(m, cv::Point(1, 1)))
				m.at<uchar>(cv::Point(1, 1)) = 40;
			else
				m.at<uchar>(cv::Point(1, 1)) = 210;

			if(isBlack(m, cv::Point(1, 1)))
				cv::rotate(m, m, cv::ROTATE_90_CLOCKWISE);
			else if (isBlack(m, cv::Point(1, 4)))
				cv::rotate(m, m, cv::ROTATE_180);
			else if (isBlack(m, cv::Point(4, 4)))
				cv::rotate(m, m, cv::ROTATE_90_COUNTERCLOCKWISE);

			int id = 0;
			if (isBlack(m, cv::Point(2, 2))) {
				id += 1;
			}
			if (!isBlack(m, cv::Point(2, 3))) {
				id += 2;
			}
			if (!isBlack(m, cv::Point(3, 3))) {
				id += 4;
			}
			if (!isBlack(m, cv::Point(3, 2))) {
				id += 8;
			}
			if (!isBlack(m, cv::Point(1, 3))) {
				id += 16;
			}
			if (!isBlack(m, cv::Point(4, 3))) {
				id += 32;
			}
			if (!isBlack(m, cv::Point(3, 1))) {
				id += 64;
			}
			if (!isBlack(m, cv::Point(3, 4))) {
				id += 128;
			}
			if (!isBlack(m, cv::Point(2, 1))) {
				id += 256;
			}
			if (!isBlack(m, cv::Point(1, 2))) {
				id += 512;
			}
			if (isBlack(m, cv::Point(2, 4))) {
				id += 1024;
			}
			if (isBlack(m, cv::Point(4, 2))) {
				id += 2048;
			}
			m_markerIDPort->send(id);

			validmarker.push_back(std::pair(id, center));
			
			cv::resize(m.getUMat(cv::ACCESS_FAST), tmp, cv::Size(250, 250), 0, 0, cv::INTER_NEAREST);
			std::stringstream strstr;
			strstr << id;

			cv::putText(canny, strstr.str(), center, 0, 1, ci::toOcv(util::Design::primaryColor()), 5);
			
			cv::cvtColor(tmp, marker, cv::COLOR_GRAY2BGR);
			cv::putText(marker, strstr.str(), cv::Point(20, 200), 0, 3, ci::toOcv(util::Design::primaryColor()), 15);
			cv::flip(marker, tmp, 1);
			m_tinyMarkerPort->send(tmp);
		}
		else {
			drawContours(canny, candidates, i, ci::toOcv(util::Design::secondaryColor()), 2, 8, hierarchy, 0, cv::Point());
		}
	}

	if (m_show) {
		m_texture = ci::gl::Texture2d::create(ci::fromOcv(canny));
	}
	

}

ci::Json act::proc::MarkerDetectionProcNode::toParams() {
	ci::Json json = ci::Json::object();
	json["resizeScale"]			= m_resizeScale;
	json["min"]					= m_min;
	json["max"]					= m_max;
	json["approximation"]		= m_approximation;
	json["distanceThreshold"]	= m_distanceThreshold;
	return json;
}

void act::proc::MarkerDetectionProcNode::fromParams(ci::Json json) {
	util::setValueFromJson(json, "resizeScale", m_resizeScale);
	util::setValueFromJson(json, "min", m_min);
	util::setValueFromJson(json, "max", m_max);
	util::setValueFromJson(json, "approximation", m_approximation);
	util::setValueFromJson(json, "distanceThreshold", m_distanceThreshold);
}

bool act::proc::MarkerDetectionProcNode::isBlack(cv::Mat mat, cv::Point pt) {
 	return mat.at<uchar>(pt) < 128;
}
