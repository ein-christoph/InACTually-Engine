
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
#include "ImageEnhancerProcNode.hpp"


act::proc::ImageEnhancerProcNode::ImageEnhancerProcNode() : ProcNodeBase("ImageEnhancer") {

	m_show = false;

	m_toGrayScale = false;
	m_equalize = false;
	m_adaptiveLuminance = true;
	m_horizontalFlip = false;
	m_verticalFlip = false;
	m_tiltLeft = false;
	m_tiltRight = false;
	m_colorInvert = false;
	m_cropT = 0.0f;
	m_cropB = 1.0f;
	m_cropL = 0.0f;
	m_cropR = 1.0f;
	m_resize = 0.8f;
	m_blur = false;
	m_blurKernel = 50;

	m_clahe = cv::createCLAHE();
	m_claheClip = 4;
	m_clahe->setClipLimit(m_claheClip);
	
	auto image = createImageInput("image", [&](cv::UMat mat) { this->onMat(mat); });
	m_imagePort = createImageOutput("enhanced image");
}

act::proc::ImageEnhancerProcNode::~ImageEnhancerProcNode() {
}

void act::proc::ImageEnhancerProcNode::update() {
}

void act::proc::ImageEnhancerProcNode::draw() {
	beginNodeDraw();

	ImGui::Checkbox("show", &m_show);
	
	if (m_show && m_texture) {
		ci::gl::pushMatrices();
		ci::gl::rotate(ci::toRadians(180.0f));
		
		glm::vec2 texSize = ci::Rectf(m_texture->getBounds()).getCenteredFit(ci::Rectf(glm::ivec2(0, 0), m_drawSize), true).getSize();
		ImGui::Image(m_texture, texSize, glm::vec2(1, 1), glm::vec2(0, 0));


		ci::gl::pushMatrices();
	}

	ImGui::SetNextItemWidth(500);
	bool sliderUsed = ImGui::SliderFloat("resize", &m_resize, 0.01f, 1.2f);

	ImGui::PushItemWidth(300);
	if(ImGui::SliderFloat("top", &m_cropT, 0.0f, 1.0f)) {
		sliderUsed = true;
		if (m_cropT > m_cropB)
			m_cropT = m_cropB;
	}
	ImGui::SameLine();
	if(ImGui::SliderFloat("bottom", &m_cropB, 0.0f, 1.0f)) {
		sliderUsed = true;
		if (m_cropT > m_cropB)
			m_cropB = m_cropT;
	}
	if (ImGui::SliderFloat("left", &m_cropL, 0.0f, 1.0f)) {
		sliderUsed = true;
		if (m_cropL > m_cropR)
			m_cropL = m_cropR;
	}
	ImGui::SameLine();
	if (ImGui::SliderFloat("right", &m_cropR, 0.0f, 1.0f)) {
		sliderUsed = true;
		if (m_cropL > m_cropR)
			m_cropR = m_cropL;
	}
	preventDrag(sliderUsed);

	ImGui::Checkbox("tilt left", &m_tiltLeft);
	ImGui::SameLine();
	ImGui::Checkbox("tilt right", &m_tiltRight);
	ImGui::SameLine();
	ImGui::Checkbox("flip horizontal", &m_horizontalFlip);
	ImGui::SameLine();
	ImGui::Checkbox("flip vertical", &m_verticalFlip);
	
	
	ImGui::Checkbox("equalize", &m_equalize);
	ImGui::SameLine();
	ImGui::Checkbox("equalize adaptively", &m_adaptiveLuminance);
	if (m_adaptiveLuminance) {
		ImGui::SameLine();
		if (ImGui::DragInt("clipLimit", &m_claheClip, 0.1, 1, 15)) {
			sliderUsed = true;
			preventDrag(sliderUsed);
			m_clahe->setClipLimit(m_claheClip);
		}
	}
	preventDrag(sliderUsed);
	
	ImGui::Checkbox("to gray-scale", &m_toGrayScale);

	ImGui::Checkbox("invert color", &m_colorInvert);

	ImGui::Checkbox("blur image", &m_blur);

	if(m_blur){
		ImGui::InputInt("blur kernel", &m_blurKernel);
	}

	ImGui::PopItemWidth();

	endNodeDraw();
}

void act::proc::ImageEnhancerProcNode::onMat(cv::UMat event) {
	cv::UMat enhanced;

	cv::resize(event, enhanced, cv::Size(event.size().width * m_resize, event.size().height * m_resize));

	int cropX = enhanced.cols * m_cropL;
	int cropW = std::max(1, (int)(enhanced.cols * m_cropR - cropX));
	int cropY = enhanced.cols * m_cropT;
	int cropH = std::max(1, (int)(enhanced.rows * m_cropB - cropY));
	enhanced = enhanced(cv::Rect(cropX, cropY, cropW, cropH));

	if (m_tiltLeft)
		cv::rotate(enhanced, enhanced, cv::ROTATE_90_COUNTERCLOCKWISE);
	if(m_tiltRight)
		cv::rotate(enhanced, enhanced, cv::ROTATE_90_CLOCKWISE);

	if(m_horizontalFlip && m_verticalFlip)
		cv::flip(enhanced, enhanced, -1);
	else if (m_horizontalFlip)
		cv::flip(enhanced, enhanced, 1);
	else if (m_verticalFlip)
		cv::flip(enhanced, enhanced, 0);
	
	
	
	if (m_adaptiveLuminance || m_equalize) {
		cv::UMat lab_image;
		try {
			
			cv::cvtColor(enhanced, lab_image, cv::COLOR_BGR2Lab);

			// Extract the L channel
			std::vector<cv::Mat> lab_planes(3);
			cv::split(lab_image, lab_planes);  // now we have the L image in lab_planes[0]

			if (m_equalize)
				cv::equalizeHist(lab_planes[0], lab_planes[0]);
			if(m_adaptiveLuminance)
				m_clahe->apply(lab_planes[0], lab_planes[0]);

			// Merge the the color planes back into an Lab image
			//enhanced.copyTo(lab_planes[0]);
			cv::UMat test;
			cv::merge(lab_planes, enhanced);

			// convert back to RGB
			cv::cvtColor(enhanced, enhanced, cv::COLOR_Lab2BGR); // this evokes an ASSERT, cannot be handled as an exception
		}
		catch (cv::Exception& e)
		{
			const char* err_msg = e.what();
			std::cout << "exception caught: " << err_msg << std::endl;
		}
	}

	if (m_blur) {
		cv::blur(enhanced, enhanced, cv::Size(m_blurKernel, m_blurKernel));
	}

	if(m_toGrayScale)
		cv::cvtColor(enhanced, enhanced, cv::COLOR_BGR2GRAY);


	if (m_colorInvert)
		bitwise_not(enhanced, enhanced);



	m_imagePort->send(enhanced);

	if (m_show) {
		m_texture = ci::gl::Texture2d::create(ci::fromOcv(enhanced));
	}
}

ci::Json act::proc::ImageEnhancerProcNode::toParams() {
	ci::Json json = ci::Json::object();
	json["resize"]				= m_resize;
	json["cropT"]				= m_cropT;
	json["cropB"]				= m_cropB;
	json["cropL"]				= m_cropL;
	json["cropR"]				= m_cropR;
	json["tiltLeft"]			= m_tiltLeft;
	json["tiltRight"]			= m_tiltRight;
	json["horizontalFlip"]		= m_horizontalFlip;
	json["verticalFlip"]		= m_verticalFlip;
	json["toGrayScale"]			= m_toGrayScale;
	json["invert"]				= m_colorInvert;
	json["equalize"]			= m_equalize;
	json["adaptiveLuminance"]	= m_adaptiveLuminance;
	json["claheClip"]			= m_claheClip;
	json["blur"]				= m_blur;
	json["blurKernelSize"]		= m_blurKernel;
	return json;
}

void act::proc::ImageEnhancerProcNode::fromParams(ci::Json json) {
	util::setValueFromJson(json, "resize", m_resize);
	util::setValueFromJson(json, "cropT", m_cropT);
	util::setValueFromJson(json, "cropB", m_cropB);
	util::setValueFromJson(json, "cropL", m_cropL);
	util::setValueFromJson(json, "cropR", m_cropR);
	util::setValueFromJson(json, "tiltLeft", m_tiltLeft);
	util::setValueFromJson(json, "tiltRight", m_tiltRight);
	util::setValueFromJson(json, "horizontalFlip", m_horizontalFlip);
	util::setValueFromJson(json, "verticalFlip", m_verticalFlip);
	util::setValueFromJson(json, "toGrayScale", m_toGrayScale);
	util::setValueFromJson(json, "invert", m_colorInvert);
	util::setValueFromJson(json, "equalize", m_equalize);
	util::setValueFromJson(json, "adaptiveLuminance", m_adaptiveLuminance);
	util::setValueFromJson(json, "blurKernelSize", m_blurKernel);
	util::setValueFromJson(json, "blur", m_blur);
	if (util::setValueFromJson(json, "claheClip", m_claheClip))
		m_clahe->setClipLimit(m_claheClip);
}
