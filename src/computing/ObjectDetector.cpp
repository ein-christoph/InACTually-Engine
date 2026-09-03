
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
#include "ObjectDetector.hpp"

#include <chrono>
using namespace std::chrono_literals;

act::comp::ObjectDetector::ObjectDetector() : DetectorBase("objectDetector") {
	initNetwork();
}

act::comp::ObjectDetector::ObjectDetector(room::CameraRoomNodeRef camera) : DetectorBase("objectDetector", camera)
{
	initNetwork();
}


act::comp::ObjectDetector::~ObjectDetector()
{

}


ci::Json act::comp::ObjectDetector::toJson() {
	auto json = ci::Json::object();


	return json;
}

void act::comp::ObjectDetector::fromJson(ci::Json json) {

}

void act::comp::ObjectDetector::refreshObjPoints()
{
	float objSize = 1.0f;
	m_objPoints = cv::Mat(4, 1, CV_32FC3);
	m_objPoints.ptr<cv::Vec3f>(0)[0] = cv::Vec3f(-objSize / 2.f, objSize / 2.f, 0);
	m_objPoints.ptr<cv::Vec3f>(0)[1] = cv::Vec3f(objSize / 2.f, objSize / 2.f, 0);
	m_objPoints.ptr<cv::Vec3f>(0)[2] = cv::Vec3f(objSize / 2.f, -objSize / 2.f, 0);
	m_objPoints.ptr<cv::Vec3f>(0)[3] = cv::Vec3f(-objSize / 2.f, -objSize / 2.f, 0);
}

void act::comp::ObjectDetector::initNetwork()
{
	// get labels of all classes
	std::string classesFile = ci::app::getAssetPath("yolov7/coco.names.txt").string();

	std::ifstream ifs(classesFile);
	std::string line;
	while (getline(ifs, line)) m_classes.push_back(line);

	std::string cfgFile;
	std::string weightsFile;

	if (m_isUsingTiny) {
		cfgFile = ci::app::getAssetPath("yolov7/yolov7-tiny.cfg").string();
		weightsFile = ci::app::getAssetPath("yolov7/yolov7-tiny.weights").string();
		m_blobSize = cv::Size(416, 416); // should come out of cfgFile
	}
	else {
		cfgFile = ci::app::getAssetPath("yolov7/yolov7.cfg").string();
		weightsFile = ci::app::getAssetPath("yolov7/yolov7.weights").string();
		m_blobSize = cv::Size(640, 640); // should come out of cfgFile
	}

	m_network = cv::dnn::readNet(cfgFile, weightsFile);

	//add this for cuda support
	// 
	m_network.setPreferableBackend(cv::dnn::DNN_BACKEND_CUDA);
	m_network.setPreferableTarget(cv::dnn::DNN_TARGET_CUDA);

	if (m_network.empty()) {
		std::ostringstream ss;
		ss << "Failed to load network with the following settings:\n";
		throw std::invalid_argument(ss.str());
	}

	std::vector<std::string> layers = m_network.getLayerNames();
	auto i = m_network.getUnconnectedOutLayers();
	m_outputLayer = layers[i[0] - 1];

	refreshObjPoints();

	// m_tracker = std::make_shared<byte_track::BYTETracker>(30, 50, 0.5f, 0.5f, 0.9f);

	m_isInitialized = true;
}

void  act::comp::ObjectDetector::detect() {

	for (auto&& it = m_newObjectOccurence.begin(); it != m_newObjectOccurence.end();) {
		auto m = it->second;
		if (!m->wasInLastFrame) {
			it = m_newObjectOccurence.erase(it);
		}
		else {
			it++;
			m->wasInLastFrame = false;
		}
	}

	//save object that were processed by a camera to not do double work
	std::vector<int> processedMarkers;

	room::CameraRoomNodeRef cameraNode = m_camera;
	act::room::CameraDeviceRef camera = cameraNode->getCamera();
	cv::UMat image = m_currentImage;
	if (image.empty())
		return;

	cv::UMat inputBlob;
	cv::dnn::blobFromImage(image, inputBlob, 1 / 255.0, m_blobSize);

	try {
		m_network.setInput(inputBlob, "data");

		bool hadDetections = m_detection.size() > 0;

		m_detection.clear();
		m_network.forward(m_detection, getOutputsNames(m_network));

		if (hadDetections && m_detection.size() == 0) // skip if there is just a blind detection
			return; 

		cv::UMat outputImage = image.clone();

		processDetection(outputImage, m_detection);

		m_feedbackImage = outputImage;
	}
	catch (cv::Exception exc) {
		CI_LOG_E("Failed to detect Objects: " << exc.what());
	}
}

std::vector<std::string> act::comp::ObjectDetector::getOutputsNames(const cv::dnn::Net& net)
{
	if (m_names.empty())
	{
		//Get the indices of the output layers, i.e. the layers with unconnected outputs
		std::vector<int> outLayers = net.getUnconnectedOutLayers();

		//get the names of all the layers in the network
		std::vector<std::string> layersNames = net.getLayerNames();

		// Get the names of the output layers in names
		m_names.resize(outLayers.size());
		for (size_t i = 0; i < outLayers.size(); ++i)
			m_names[i] = layersNames[outLayers[i] - 1];
	}
	return m_names;
}

void act::comp::ObjectDetector::processDetection(cv::UMat& frame, const std::vector<cv::Mat>& outs)
{
	std::vector<int> classIDs;
	std::vector<float> confidences;
	std::vector<cv::Rect> boxes;

	for (size_t i = 0; i < outs.size(); ++i)
	{
		// Scan through all the bounding boxes output from the network and keep only the
		// ones with high confidence scores. Assign the box's class label as the class
		// with the highest score for the box.
		float* data = (float*)outs[i].data;
		for (int j = 0; j < outs[i].rows; ++j, data += outs[i].cols)
		{
			cv::Mat scores = outs[i].row(j).colRange(5, outs[i].cols);
			cv::Point classIDPoint;
			double confidence;
			// Get the value and location of the maximum score
			minMaxLoc(scores, 0, &confidence, 0, &classIDPoint);

			if (confidence > m_minConfidence)
			{
				int centerX = (int)(data[0] * frame.cols);
				int centerY = (int)(data[1] * frame.rows);
				int width = (int)(data[2] * frame.cols);
				int height = (int)(data[3] * frame.rows);
				int left = centerX - width / 2;
				int top = centerY - height / 2;

				classIDs.push_back(classIDPoint.x);
				confidences.push_back((float)confidence);
				boxes.push_back(cv::Rect(left, top, width, height));
			}
		}
	}

	// Perform non maximum suppression to eliminate redundant overlapping boxes with
	// lower confidences
	std::vector<int> indices;
	cv::dnn::NMSBoxes(boxes, confidences, 0.5, 0.4, indices);

	//std::vector<int> msgLabel;
	//std::vector<float> msgConf;
	//std::vector<cv::Rect> msgBoxes;
	std::vector<ObjectCandidate> candidates;

	for (size_t i = 0; i < indices.size(); ++i)
	{
		int idx = indices[i];
		cv::Rect box = boxes[idx];

		if (!m_classes.empty() && idx < (int)m_classes.size())
		{
			auto candidate = ObjectCandidate();
			candidate.id = idx;
			candidate.className = m_classes[classIDs[idx]];
			candidate.classID = classIDs[idx];
			candidate.confidence = confidences[idx];
			candidate.box = ci::Rectf(box.x, box.y, box.x + box.width, box.y + box.height);

			candidates.push_back(candidate);

			//drawBox(m_classes[classIDs[idx]], confidences[idx], box.x, box.y, box.x + box.width, box.y + box.height, frame);

			//msgLabel.push_back(classIDs[idx]);
			//msgConf.push_back(confidences[idx]);
			//msgBoxes.push_back(boxes[idx]);
		}
	}

	if (candidates.size() == 0)
		return;

	trackObjects(candidates);


	for (auto&& candidate : m_currentCandidates) {
		std::stringstream strstr;
		strstr << candidate.id << " - " << candidate.className << "[" << candidate.classID << "]";
		drawBox(strstr.str(), candidate.confidence, candidate.box.x1, candidate.box.y1,
			candidate.box.x2, candidate.box.y2, frame);
	}
		
	m_areNewCandidatesAvailable = true;
}

void act::comp::ObjectDetector::drawBox(std::string className, float conf, int left, int top, int right, int bottom, cv::UMat& frame)
{
	//Draw a rectangle displaying the bounding box
	rectangle(frame, cv::Point(left, top), cv::Point(right, bottom), ci::toOcv(util::Design::primaryColor()), 3);

	//Get the label for the class name and its confidence
	std::string label = cv::format("%.2f", conf);
	label = className + ":" + label;

	//Display the label at the top of the bounding box
	int baseLine;
	cv::Size labelSize = getTextSize(label, cv::FONT_HERSHEY_SIMPLEX, 0.5, 1, &baseLine);
	top = cv::max(top, labelSize.height);
	rectangle(frame, cv::Point(left, top - round(1.5 * labelSize.height)), cv::Point(left + round(1.5 * labelSize.width), top + baseLine), cv::Scalar(255, 255, 255), cv::FILLED);
	putText(frame, label, cv::Point(left, top), cv::FONT_HERSHEY_SIMPLEX, 0.75, cv::Scalar(0, 0, 0), 1);
}

void act::comp::ObjectDetector::checkOccurency(int id)
{
	bool isNew = true;
	for (auto&& occurence : m_newObjectOccurence) {
		auto&& m = occurence.second;
		if (m->id == id) {
			isNew = false;

			m->occurence++;
			m->wasInLastFrame = true;

			if (m->occurence > 10) {
				m->wasInLastFrame = false;
				m_validObjectIDs[m->id] = true;
			}

			break;
		}
	}
	if (isNew) {
		m_newObjectOccurence[id] = ObjectOccurence::create(id);
	}
}


void act::comp::ObjectDetector::trackObjects(std::vector<ObjectCandidate>& candidates)
{
	/*
	std::vector<byte_track::Object> objects;

	for (auto&& c : candidates) {
		auto rect = byte_track::Rect<float>(c.box.getX1(), c.box.getY1(), c.box.getWidth(), c.box.getHeight());
		byte_track::Object obj(rect, c.classID, c.confidence);
		objects.push_back(obj);
	}
	
	const auto results = m_tracker->update(objects);
	
	candidates.clear();
	for (auto&& r : results) {
		auto candidate = ObjectCandidate();
		candidate.id = r->getTrackId();
		candidate.confidence = r->getScore();
		auto rect = r->getRect();
		candidate.box = ci::Rectf(rect.tl_x(), rect.tl_y(), rect.br_x(), rect.br_y());
		candidate.classID = r->getLabel();
		candidate.className = m_classes[candidate.classID];
		candidates.push_back(candidate);
	}*/

	/*if (results.size() != candidates.size()) {
		m_currentCandidates.resize(0);
		return;
	}

	for (int i = 0; i < results.size() && i < candidates.size(); i++) {
		checkOccurency(results[i]->getTrackID());
		if (!m_validObjectIDs[results[i]->getTrackID()]) {
			candidates.erase(candidates.begin() + i);
			continue;
		}

		candidates[i].id = results[i]->getTrackID();
		auto r = results[i]->getRect();
		candidates[i].box = ci::Rectf(r.tl_x(), r.tl_y(), r.br_x(), r.br_y());
	}*/

	m_currentCandidates = candidates;
}
