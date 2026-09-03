
/*
	InACTually
	> interactive theater for actual acts
	> this file is part of the "InACTually Engine", a MediaServer for driving all technology

	Copyright (c) 2021–2025 Lars Engeln, Fabian Töpfer
	Copyright (c) 2025 InACTually Community
	Licensed under the MIT License.
	See LICENSE file in the project root for full license information.

	This file is created and substantially modified: 2022-2023

	contributors:
	Lars Engeln - mail@lars-engeln.de
*/

#include "roompch.hpp"
#include "kinect/KinectManager.hpp"

#include "kinect/KinectRoomNode.hpp"
#include "kinect/KinectDummy.hpp"

act::room::KinectManager::KinectManager() : RoomNodeManagerBase("kinectManager")
{
	m_selectedDevice = 0;
	refreshLists();

	//m_updateThread = std::thread([&]() { while (1) { updateKinects(); }; });
}

act::room::KinectManager::~KinectManager()
{
	//m_updateThread.join();
	stopDevices();
}

void act::room::KinectManager::setup()
{
}
void act::room::KinectManager::update() {
	m_devicesAndStates.clear();
	for (const auto& deviceName : m_usedDeviceNames) {
		checkConnectionState(deviceName);
	}

	m_disconnectedDeviceNames.clear();
	for (const auto& kinectNode : m_nodes) {
		std::string stateStr;
		if (!kinectNode->isConnected())
			m_devicesAndStates.insert(std::make_pair(kinectNode->getName(), K4_DISCONNECTED));
	}

	if (m_isDummyOpenDialog) {
		m_isDummyOpenDialog = false;
		std::vector<std::string> exts;
		exts.push_back("json");
		std::string path = ci::app::getOpenFilePath(ci::app::getAssetPath("./../recordings/"), exts).string();
		addDummyDevice(path);
	}	

	updateKinects();
}
void act::room::KinectManager::updateKinects()
{
	for (auto&& node : m_nodes) {
		node->update();
	}

	if (m_devices.size() > 0)
	{
		m_bodiesMerged.clear();
		m_bodiesWithDuplicates.clear();

		for (auto&& device : m_devices) {
			checkRedundantBodies(device->getRepositionedBodies());
		}
	}
}

void act::room::KinectManager::draw()
{
	//drawBodySkeletons(m_bodiesWithDuplicates);

	int k_body = 1;

	if (m_colorMapping.size() > 0)
	{
		for (auto&& device : m_devices) {
			std::map<uint32_t, k4abt_skeleton_t> currBodies = device->getRepositionedBodies();

			util::ColorGradient colorGrad;

			colorGrad.add(ci::ColorA(1, 1, 1, 1), 1.0f);

			if (m_colorMapping.size() >= k_body) {
				for (int i = 0; i < currBodies.size(); i++)
				{
					colorGrad.add(m_colorMapping.at(k_body), i / 255.0f);
					k_body++;
				}
			}
			device->setColorMap(colorGrad.toColorMap(colorGrad));
		}
	}

	for (auto&& node : m_nodes) {
		node->draw();
	}
}

act::room::RoomNodeBaseRef act::room::KinectManager::drawMenu()
{
	ImGui::Combo("Azure Kinect", &m_selectedDevice, m_availableDeviceNames);

	bool noDevicesFound = m_availableDeviceNames.empty();

	if (!m_devicesAndStates.empty()) {
		ImGui::Text("Known Devices:");

		for (const auto& value : m_devicesAndStates) {

			act::room::kinectConnectionState currState = value.second;
			std::string connectionStr;
			ImVec4 col;

			switch (currState)
			{
			case act::room::K4_CONNECTED:
				connectionStr = "connected";
				col = act::util::Design::highlightColor();
				break;
			case act::room::K4_NETWORK_CONNECTED:
				connectionStr = "connected(Net)";
				col = act::util::Design::highlightColor();
				break;
			case act::room::K4_DISCONNECTED:
				connectionStr = "disconnected";
				col = act::util::Design::errorColor();
				break;
			default:
				connectionStr = "disconnected";
				col = act::util::Design::errorColor();
				break;
			}

			ImGui::BulletText("%s ", value.first.c_str());

			ImGui::SameLine();

			connectionStr = "[" + connectionStr + "]";

			ImGui::TextColored(col, connectionStr.c_str());
		}
	}

	ImGui::NewLine();

	ImGui::Separator();

	if (noDevicesFound) {
#ifdef WITHKINECT

		if (m_usedDeviceNames.size() == 0)
			ImGui::Text("No Azure KinectDevice device has been found.");
		else
			ImGui::Text("All available Azure KinectDevice devices are already in use.");
#else
		ImGui::Text("This Version was compiled without KinectDevice support.");
#endif
	}
	else {
		if (ImGui::Button("add Device")) {
			return addDevice(m_availableDeviceNames[m_selectedDevice], m_availableDeviceNames[m_selectedDevice]); // add input for name
		}
		ImGui::NewLine();
		ImGui::Checkbox("CPU Mode (if no CUDA available)", &cpu_mode);
	}
	if (ImGui::Button("add Dummy-Device")) {
		m_isDummyOpenDialog = true;
	}
	if (m_dummyDevice) { // collect dummy device
		auto tmp = m_dummyDevice;
		m_dummyDevice = nullptr;
		return tmp;
	}

	ImGui::NewLine();

	ImGui::Separator();

	ImGui::Text("Body Merging");

	ImGui::SliderFloat("Body Merge Threshold (Meter)", &m_bodyClippingThreshold, 0.0f, 2.0f);

	ImGui::Checkbox("Simple Merge Mode", &simpleMergeMode);


	ImGui::NewLine();

	ImGui::Separator();

	ImGui::Text("PointCloud Registration");

	if (ImGui::Button("register")) {
		//performPointCloudRegistration();
		return nullptr;
	}


	return nullptr;
}

ci::Json act::room::KinectManager::toJson()
{
	auto json = ci::Json::object();

	ci::Json nodes = ci::Json::array();
	for (auto&& node : m_nodes) {
		auto nodeJson = node->toJson();
		nodes.push_back(nodeJson);
	}

	ci::Json availableDevices = ci::Json::array();
	for (auto&& name : m_availableDeviceNames) {
		auto deviceJson = ci::Json::object();
		deviceJson["deviceName"] = name;
		availableDevices.push_back(deviceJson);
	};
	
	json["availableDevices"] = availableDevices;

	json["nodes"] = nodes;

	return json;
}

void act::room::KinectManager::fromJson(ci::Json json)
{
	m_devicesAndStates.clear();

	if (json.contains("nodes")) {
		auto nodesJson = json["nodes"];
		for (auto&& node : nodesJson) {

			std::string name = "";
			util::setValueFromJson(node, "name", name);
			if (name.substr(0, 5) != "Dummy") {

				//check if device is connected
				//if not, set disconnected

				auto kinectDevice = addDevice(name, name);

				kinectDevice->fromJson(node);
				kinectDevice->setIsConnected(checkConnectionState(name));
			}
		}
	}
}

act::proc::ImageOutputPortRef act::room::KinectManager::getKinectPort(std::string kinectName, kinectImagePort port)
{
	auto nodeIter = std::find_if(m_nodes.begin(), m_nodes.end(),
		[kinectName](RoomNodeBaseRef node) {
			return !node->getName().compare(kinectName);
		});


	if (nodeIter != std::end(m_nodes)) {
		KinectRoomNodeRef kinect = std::dynamic_pointer_cast<KinectRoomNode> (*nodeIter);
		if (kinect)
			switch (port)
			{
			case act::room::KP_IMAGE:
				return kinect->getKinectImagePort();
			case act::room::KP_DEPTH:
				return kinect->getKinectDepthPort();
			case act::room::KP_INFRARED:
				return kinect->getKinectIRPort();
			case act::room::KP_BIM:
				return kinect->getKinectBIMPort();
			default:
				break;
			}
	}
	return nullptr;
}

act::proc::OutputPortRef<act::room::BodyRefList> act::room::KinectManager::getBodyPortByName(std::string kinectName)
{
	auto nodeIter = std::find_if(m_nodes.begin(), m_nodes.end(),
		[kinectName](RoomNodeBaseRef node) {
			return !node->getName().compare(kinectName);
		});
	if (nodeIter != std::end(m_nodes)) {
		KinectRoomNodeRef kinect = std::dynamic_pointer_cast<KinectRoomNode> (*nodeIter);
		if (kinect)
			return kinect->getBodyPort();
	}
	return nullptr;
}

act::proc::OutputPortRef<act::room::BodyRefList> act::room::KinectManager::getBodyPort(act::UID uid)
{
	auto nodeIter = std::find_if(m_nodes.begin(), m_nodes.end(),
		[uid](RoomNodeBaseRef node) {
			return !node->getUID().compare(uid);
		});
	if (nodeIter != std::end(m_nodes)) {
		KinectRoomNodeRef kinect = std::dynamic_pointer_cast<KinectRoomNode> (*nodeIter);
		if (kinect)
			return kinect->getBodyPort();
	}
	return nullptr;
}

void act::room::KinectManager::refreshLists()
{
	stopDevices();

	m_devices = getDevices();
	for (auto&& kinect : m_devices) {
		if (std::find_if(m_nodes.begin(), m_nodes.end(), [&](RoomNodeBaseRef node) { return node->getName() == kinect->getName(); }) == m_nodes.end())
			m_availableDeviceNames.push_back(kinect->getName());
	}

	m_usedDeviceNames.clear();
	for (auto&& kinect : m_nodes) {
		m_usedDeviceNames.push_back(kinect->getName());
	}
}

std::vector<act::room::KinectDeviceRef> act::room::KinectManager::getDevices()
{
	std::vector <act::room::KinectDeviceRef> kinectDevices;
#ifdef WITHKINECT
	m_devicesInstalledCount = k4a::device::get_installed_count();
#endif
	if (m_devicesInstalledCount == 0) {
		m_availableDeviceNames.clear();
		CI_LOG_I("No kinect device available!");
	}
	else {
		for (int deviceIndex = 0; deviceIndex < m_devicesInstalledCount; deviceIndex++) {

			act::room::KinectDeviceRef ref = act::room::KinectDevice::create(deviceIndex);
			kinectDevices.push_back(ref);
		}
	}

	return kinectDevices;
}

act::room::KinectDeviceRef act::room::KinectManager::getDeviceByName(std::string deviceName)
{
	for (int i = 0; i < m_devices.size(); i++) {
		if (m_devices[i]->getName() == deviceName) {
			return m_devices[i];
		}
	}
	return NULL;
}

glm::vec3 act::room::KinectManager::getKinectPositionByUID(act::UID uid)
{
	glm::vec3 pos = glm::vec3(0.0f);
	auto kinect = getNodeByUID(uid);
	if (kinect)
		pos = kinect->getPosition();
	return pos;
}

glm::vec3 act::room::KinectManager::getKinectOrientationByUID(act::UID uid)
{
	glm::vec3 rot = glm::vec3(0.0f);
	auto kinect = getNodeByUID(uid);
	if (kinect)
		rot = kinect->getRotation();
	return rot;
}

act::room::RoomNodeBaseRef act::room::KinectManager::addDevice(std::string deviceName, std::string name)
{
	KinectDeviceRef newDevice;

	bool foundDevice = false;
	for (const auto& device : m_devices) {
		if (device->getName() == deviceName) {
			foundDevice = true;
			newDevice = device;
			break;
		}
	}

	if (foundDevice) {
		try {
			newDevice->startDevice(cpu_mode);
		}
		catch (...) {
			CI_LOG_E("Failed to start kinect device");
		}

		auto kinect = KinectRoomNode::create(newDevice, deviceName, name);
		m_nodes.push_back(kinect);

		deviceName = kinect->getName();

		m_usedDeviceNames.push_back(deviceName);

		m_availableDeviceNames.erase(std::remove(m_availableDeviceNames.begin(), m_availableDeviceNames.end(), deviceName), m_availableDeviceNames.end());


		//refreshDeviceList();
		return kinect;

	}
	else { // if device not available anymore, i.e. loading Nodes
		CI_LOG_E("Cannot find KinectDevice " << deviceName << "!");
		auto kinect = KinectRoomNode::create(nullptr, deviceName, name);
		m_nodes.push_back(kinect);
		return kinect;
		//refreshDeviceList();
	}
}


/*
	Kinect recording .json description
	
	recording => mame of mkv file parallel to json file
	position,
	rotation,
	...


*/
void act::room::KinectManager::addDummyDevice(std::string path)
{
	ci::Json data = ci::loadJson(ci::loadFile(path));
	std::string filename = "";

 	int namePos = path.rfind("\\");
	
	
	if (data.contains("recording")) {
		filename = data["recording"];
		filename = path.substr(0, namePos) + "\\" + filename;
	};

	if (filename == "") {
		//LOG ERROR NOT FOUND 
		return;
	};

	KinectDummyRef device = KinectDummy::create(-1, filename, filename, data["bodyFrames"]);

	m_devices.push_back(device);
	m_usedDeviceNames.push_back(device->getName());

	glm::vec3 pos = glm::vec3(0.0f);
	if (data.contains("position")) {
		pos.x = data["position"]["x"];
		pos.y = data["position"]["y"];
		pos.z = data["position"]["z"];
	}
	glm::vec3 rot = glm::vec3(0.0f);
	if (data.contains("rotation")) {
		rot.x = data["rotation"]["x"];
		rot.y = data["rotation"]["y"];
		rot.z = data["rotation"]["z"];
	}

	m_dummyDevice = KinectRoomNode::create(device, filename, filename, pos, rot);
	m_nodes.push_back(m_dummyDevice);
}

void act::room::KinectManager::stopDevices()
{
	if (m_devices.size() > 0)
	{
		for(auto&& device:m_devices){
			device->stopDevice();
		}
		m_devices.clear();
	}
}
/*
void act::room::KinectManager::drawBodySkeletons(std::map<uint32_t, std::pair<k4abt_skeleton_t, int>> bodyMap)
{
	bool isFirstBody = true;

	ci::gl::ScopedLineWidth scpLW(5);

	int size = bodyMap.size();

	for (auto&& body : bodyMap)
	{
		uint32_t bodyID = body.first;
		k4abt_skeleton_t skeleton = body.second.first;
		uint32_t duplicateRef = (uint32_t)body.second.second;

		if (isFirstBody)
		{
			if (m_colorMapping.count(bodyID - 1) > 0)
			{
				m_colorMapping.erase(bodyID - 1);
			}
			isFirstBody = false;
		}

		if (m_colorMapping.count(bodyID) >= 0)
		{
			if (duplicateRef == 0)
				m_colorMapping[(int)bodyID] = colorGenerator((int)bodyID);
			else
				m_colorMapping[(int)bodyID] = m_colorMapping[duplicateRef];
		}

		if (duplicateRef == 0)
		{
			auto color = m_colorMapping[bodyID];
			ci::gl::ScopedColor scpColor(color);

			for (int i = 0; i < 32; i++)
			{
				k4a_float3_t childJoint = skeleton.joints[i].position;
				glm::vec3 child = glm::vec3(childJoint.xyz.x, childJoint.xyz.y, childJoint.xyz.z);
				switch (skeleton.joints[i].confidence_level) {
				case K4ABT_JOINT_CONFIDENCE_NONE:
					ci::gl::color(ci::Color::gray(0.5f));
					ci::gl::lineWidth(2);

					break;
				case K4ABT_JOINT_CONFIDENCE_LOW:
					ci::gl::color(ci::ColorA(color.r, color.g, color.b, 0.6f));
					ci::gl::lineWidth(4);
					break;
				default: // case K4ABT_JOINT_CONFIDENCE_MEDIUM:

					break;
				}

				ci::gl::drawCube(child, glm::vec3(0.05f));

				k4a_float3_t::_xyz parentJoint = skeleton.joints[jointParentLookUp[i]].position.xyz;
				glm::vec3 parent = glm::vec3(parentJoint.x, parentJoint.y, parentJoint.z);

				ci::gl::drawLine(child, parent);
			}
		}
	}
	ci::gl::lineWidth(1);
}

ci::ColorA act::room::KinectManager::colorGenerator(int id, float alpha)
{
	float hOffset = fmod(id * COLOROFFSET, 1);
	ci::Color rgb = ci::hsvToRgb(glm::vec3(hOffset, 1.0f, 0.5f));

	return ci::ColorA(rgb.r, rgb.g, rgb.b, std::clamp(alpha, 0.0f, 1.0f));
}*/

act::room::PointcloudRoomNodeRef act::room::KinectManager::createPointcloudRoomNode()
{
	auto pointcloud = room::PointcloudRoomNode::create(glm::vec3(0,0,0), 1.0f, "pointcloud");
	m_nodes.push_back(pointcloud);
	return pointcloud;
}

act::room::BodyRefList act::room::KinectManager::getBodies()
{
	room::BodyRefList bodies = kh::toGenericBody(m_bodiesMerged);

	// TODO map uid

	return bodies;
}

void act::room::KinectManager::checkRedundantBodies(std::map<uint32_t, k4abt_skeleton_t> currBodies)
{
	if (m_bodiesMerged.size() == 0) {
		for (auto&& currBody : currBodies) {
			m_bodiesMerged.insert(std::make_pair(uint32_t(m_bodiesMerged.size() + 1), currBody.second));
			m_bodiesWithDuplicates.insert(std::make_pair((m_bodiesWithDuplicates.size() + 1), std::make_pair(currBody.second, 0)));
		}
	}
	else {
		for (auto&& currBody : currBodies)
		{
			k4abt_skeleton_t currSkeleton = currBody.second;
			k4abt_skeleton_t s_currSkeleton;
			uint32_t s_currBodyID;

			int duplicateRef = 0;

			for (auto&& s_currBody : m_bodiesMerged)
			{
				s_currSkeleton = s_currBody.second;
				s_currBodyID = s_currBody.first;

				//check for specific joint position
				int joint = K4ABT_JOINT_SPINE_CHEST;

				float min_distance = 1000.0f;

				for (int i = 0; i < 32; i++) {
					if (simpleMergeMode) {
						i = K4ABT_JOINT_SPINE_CHEST;
					}
					if (s_currSkeleton.joints[i].confidence_level == k4abt_joint_confidence_level_t::K4ABT_JOINT_CONFIDENCE_MEDIUM)
						if (s_currSkeleton.joints[i].confidence_level == currSkeleton.joints[i].confidence_level) {

							k4a_float3_t::_xyz s_pos = s_currSkeleton.joints[i].position.xyz;
							k4a_float3_t::_xyz pos = currSkeleton.joints[i].position.xyz;

							glm::vec3 center = glm::vec3(s_pos.x, s_pos.y, s_pos.z);
							glm::vec3 point = glm::vec3(pos.x, pos.y, pos.z);

							float distance = hypot(hypot(center.x - point.x, center.y - point.y), center.z - point.z);

							if (distance < min_distance)
								min_distance = distance;
						}

					if (simpleMergeMode) {
						break;
					}
				}

				if (min_distance < m_bodyClippingThreshold) {
					duplicateRef = (int)s_currBodyID;
				}
			}

			if (duplicateRef == 0)
			{
				m_bodiesMerged.insert(std::make_pair(uint32_t(m_bodiesMerged.size() + 1), currSkeleton));
			}

			m_bodiesWithDuplicates.insert(std::make_pair(uint32_t(m_bodiesWithDuplicates.size() + 1), std::make_pair(currSkeleton, duplicateRef)));
		}
	}
}

bool act::room::KinectManager::checkConnectionState(std::string kinectName) {

	act::room::kinectConnectionState currState;

	if (std::find(m_usedDeviceNames.begin(), m_usedDeviceNames.end(), kinectName) != m_usedDeviceNames.end())
		//TODO: Check if connected via network?
		if (false) {
			currState = K4_NETWORK_CONNECTED;
		}
		else {
			currState = K4_CONNECTED;
		}
	else
		currState = K4_DISCONNECTED;

	m_devicesAndStates.insert(std::make_pair(kinectName, currState));

	switch (currState)
	{
	case act::room::K4_CONNECTED:
		return true;
		break;
	case act::room::K4_NETWORK_CONNECTED:
		return true;
		break;
	case act::room::K4_DISCONNECTED:
		return false;
		break;
	default:
		return false;
		break;
	}
}

bool act::room::KinectManager::hasAvailableDevice(std::string captureName) {

	for (auto&& d : m_availableDeviceNames) {
		if (d.compare(captureName) == 0) {
			return true;
		}
	}

	return false;
}
/*
void	act::room::KinectManager::performPointCloudRegistration() {
	// call register point clouds
	if (m_nodes.size() > 0) {

		auto targetKinect = std::dynamic_pointer_cast<act::room::KinectRoomNode>(m_nodes[0]);
		auto targetPointCloud = targetKinect->getWorldSpacePointCloud();

		for (int i = 0; i < m_nodes.size(); i++) {
			if (i > 0) {
				auto sourceKinect = std::dynamic_pointer_cast<act::room::KinectRoomNode>(m_nodes[i]);
				auto sourcePointCloud = sourceKinect->getWorldSpacePointCloud();
				Eigen::Matrix4f transformation = registerPointClouds(sourcePointCloud, targetPointCloud);
				
				//set transformation of room node

				glm::vec3 translation(transformation(0,3), transformation(1, 3), transformation(2, 3));
				sourceKinect->setPosition(sourceKinect->getPosition() + translation);


				glm::mat3  rotationMat = glm::mat3{
				transformation(0,0), transformation(0, 1), transformation(0, 2),
				transformation(1,0), transformation(1, 1), transformation(1, 2),
				transformation(2,0), transformation(2, 1), transformation(2, 2)
				};

				glm::quat orientation = glm::quat_cast(rotationMat);
				
				sourceKinect->setOrientation(orientation * sourceKinect->getOrientation());
			}
		}
	}
}

Eigen::Matrix4f	act::room::KinectManager::registerPointClouds(act::room::Pointcloud source, act::room::Pointcloud target) {
	
	//convert PointXYZRGB => PointXYZ

 
	auto sourceConverted = pcl::PointCloud<pcl::PointXYZ>();
	pcl::copyPointCloud(*source, sourceConverted);
	auto sourceConvertedPtr = pcl::make_shared<pcl::PointCloud<pcl::PointXYZ>>(sourceConverted);
	sourceConvertedPtr = reducePointCloudSize(sourceConvertedPtr);

	/*auto sourceConvertedPtr = pcl::make_shared<pcl::PointCloud<pcl::PointXYZ>>();
	sourceConvertedPtr->push_back(pcl::PointXYZ(1.001f,1.0f,1.0f));
	sourceConvertedPtr->push_back(pcl::PointXYZ(1.002f, 1.0f, 1.0f));
	sourceConvertedPtr->push_back(pcl::PointXYZ(1.003f, 1.0f, 1.0f));*/
	 

/*
	auto targetConverted = pcl::PointCloud<pcl::PointXYZ>();
	pcl::copyPointCloud(*target, targetConverted);
	auto targetConvertedPtr = pcl::make_shared<pcl::PointCloud<pcl::PointXYZ>>(targetConverted);
	
	targetConvertedPtr = reducePointCloudSize(targetConvertedPtr);

	/*auto targetConvertedPtr = pcl::make_shared<pcl::PointCloud<pcl::PointXYZ>>();
	targetConvertedPtr->push_back(pcl::PointXYZ(1.000f, 1.0f, 1.0f));
	targetConvertedPtr->push_back(pcl::PointXYZ(1.001f, 1.0f, 1.0f));
	targetConvertedPtr->push_back(pcl::PointXYZ(1.002f, 1.0f, 1.0f));*/

/*
	pcl::IterativeClosestPoint<pcl::PointXYZ, pcl::PointXYZ> icp;
	// Set the input source and target
 	icp.setInputSource(sourceConvertedPtr);
	icp.setInputTarget(targetConvertedPtr);

	// Set the max correspondence distance to 5cm (e.g., correspondences with higher
	// distances will be ignored)
	icp.setMaxCorrespondenceDistance(0.05);
	// Set the maximum number of iterations (criterion 1)
	icp.setMaximumIterations(50);
	// Set the transformation epsilon (criterion 2)
	icp.setTransformationEpsilon(1e-8);
	// Set the euclidean distance difference epsilon (criterion 3)
	icp.setEuclideanFitnessEpsilon(1);

	// Perform the alignment
		
	pcl::PointCloud<pcl::PointXYZ>::Ptr source_aligned(new    pcl::PointCloud<pcl::PointXYZ>());

	icp.align(*source_aligned);

	// Obtain the transformation that aligned cloud_source to cloud_source_registered
	Eigen::Matrix4f transformation = icp.getFinalTransformation();

	 
	CI_LOG_I("Transformation is:\n"
		"\t " << transformation(0, 0) << "\t " << transformation(0, 1) << "\t " << transformation(0, 2) << "\t " << transformation(0, 3) << "\n"
		"\t " << transformation(1, 0) << "\t " << transformation(1, 1) << "\t " << transformation(1, 2) << "\t " << transformation(1, 3) << "\n"
		"\t " << transformation(2, 0) << "\t " << transformation(2, 1) << "\t " << transformation(2, 2) << "\t " << transformation(2, 3) << "\n"
		"\t " << transformation(3, 0) << "\t " << transformation(3, 1) << "\t " << transformation(3, 2) << "\t " << transformation(3, 3) << "\n");
 
	return transformation;
}	

pcl::PointCloud<pcl::PointXYZ>::Ptr act::room::KinectManager::reducePointCloudSize(pcl::PointCloud<pcl::PointXYZ>::Ptr source) {
	CI_LOG_I("PointCloud Reduction before" << source->size());
	
	pcl::PointCloud<pcl::PointXYZ>::Ptr cloud_filtered(new pcl::PointCloud<pcl::PointXYZ>);
	pcl::PointCloud<pcl::PointXYZ>::Ptr cloud_downSampled(new pcl::PointCloud<pcl::PointXYZ>);
	pcl::PointCloud<pcl::PointXYZ>::Ptr cloud_final(new pcl::PointCloud<pcl::PointXYZ>);

	pcl::PassThrough<pcl::PointXYZ> pass;
    pass.setInputCloud(source);
	pass.setFilterFieldName("z");
	pass.setFilterLimits(0.1, 5.0);
 	pass.filter(*cloud_filtered);
	CI_LOG_I("PointCloud Reduction passthrough filter" << cloud_filtered->size());


	pcl::VoxelGrid<pcl::PointXYZ> avg;
	avg.setInputCloud(cloud_filtered);
	avg.setLeafSize(0.005f, 0.005f, 0.005f);
	avg.filter(*cloud_downSampled);
	CI_LOG_I("PointCloud Reduction downsampled" << cloud_downSampled->size());

	pcl::StatisticalOutlierRemoval<pcl::PointXYZ> sor;
	sor.setInputCloud(cloud_downSampled);
	sor.setMeanK(50);
	sor.setStddevMulThresh(1.0);
	sor.filter(*cloud_final);

	CI_LOG_I("PointCloud Reduction after" << cloud_final->size());
	return cloud_final;
}
*/