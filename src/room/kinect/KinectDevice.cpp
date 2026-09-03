
/*
	InACTually
	> interactive theater for actual acts
	> this file is part of the "InACTually Engine", a MediaServer for driving all technology

	Copyright (c) 2021–2025 Lars Engeln, Fabian Töpfer
	Copyright (c) 2025 InACTually Community
	Licensed under the MIT License.
	See LICENSE file in the project root for full license information.

	This file is created and substantially modified: 2022-2025

	contributors:
	Lars Engeln - mail@lars-engeln.de
*/

#include "roompch.hpp"
#include "kinect/KinectDevice.hpp"

using namespace cinder::app;
using namespace act;
using namespace room;

KinectDevice::KinectDevice(std::uint32_t deviceID) : m_deviceIndex(deviceID), m_nodeUID("")
{
	initialize();
	openDevice();
}

KinectDevice::~KinectDevice()
{
	stopDevice();
}

void KinectDevice::initialize()
{
	m_isProvidingPointCloud = false;
	util::ColorGradient colorGrad;

	colorGrad.add(ci::ColorA(0, 0, 0, 1), 0.0f);
	colorGrad.add(ci::ColorA(1, 1, 1, 1), 1.0f);
	setColorMap(colorGrad.toColorMap(colorGrad));

#ifdef WITHKINECT
	m_deviceConfiguration = K4A_DEVICE_CONFIG_INIT_DISABLE_ALL;
	m_deviceConfiguration.color_format = k4a_image_format_t::K4A_IMAGE_FORMAT_COLOR_BGRA32;// K4A_IMAGE_FORMAT_COLOR_MJPG; 
	m_deviceConfiguration.color_resolution = k4a_color_resolution_t::K4A_COLOR_RESOLUTION_720P; //K4A_COLOR_RESOLUTION_1536P;

	m_deviceConfiguration.camera_fps = K4A_FRAMES_PER_SECOND_30;

	m_deviceConfiguration.depth_mode = k4a_depth_mode_t::K4A_DEPTH_MODE_NFOV_UNBINNED;
	m_deviceConfiguration.synchronized_images_only = false;
	m_deviceConfiguration.wired_sync_mode = k4a_wired_sync_mode_t::K4A_WIRED_SYNC_MODE_STANDALONE;
#endif

	m_isColorFrameAvailable = true;
	m_isDepthFrameAvailable = true;
	m_isInfraRedFrameAvailable = true;
	m_isBodyIndexFrameAvailable = true;
	m_areBodiesAvailable = true;
	m_isPointCloudAvailable = false;
}

void KinectDevice::openDevice()
{
	if (m_state == KinectState::KS_CLOSED) {
#ifdef WITHKINECT
		try {
			m_device = k4a::device::open(0);
		}
		catch (k4a::error err) {
			CI_LOG_E("Failed to open kinect.");
		}
#endif;
		getSerialNumber();

		try {
			m_sensorCalibration = m_device.get_calibration(m_deviceConfiguration.depth_mode, m_deviceConfiguration.color_resolution);
			m_transformation = k4a::transformation::transformation(m_sensorCalibration);
		}
		catch(k4a::error err) {
			CI_LOG_E("Failed to get calibration");
		}

		m_state = KinectState::KS_OPENED;
	}
	else {
		CI_LOG_E("Failed to open the kinect device! Probably already opened (and capturing).");
	}
}

/*
Pointcloud act::room::KinectDevice::toPointCloud(k4a::image depth, k4a::image color)
{
	Pointcloud pointcloud

	bool noci::Color = color == nullptr;

	int width = k4a_image_get_width_pixels(depth);
	int height = k4a_image_get_height_pixels(depth);

	if (width == 0 || height == 0) {
		return pointcloud;
	}

	k4a::image transformedci::Color = NULL;
	if (K4A_RESULT_SUCCEEDED != k4a_image_create(K4A_IMAGE_FORMAT_COLOR_BGRA32,
		width,
		height,
		0,
		&transformedColor))
	{
		CI_LOG_E("Failed to create transformed color image\n");
		return pointcloud;
	}

	k4a::image pointcloudImage = NULL;
	if (K4A_RESULT_SUCCEEDED != k4a_image_create(K4A_IMAGE_FORMAT_CUSTOM,
		width,
		height,
		width * 3 * (int)sizeof(int16_t),
		&pointcloudImage))
	{
		CI_LOG_E("Failed to create point cloud image\n");
		return pointcloud;
	}

	if (!noci::Color && K4A_RESULT_SUCCEEDED != k4a_transformation_color_image_to_depth_camera(m_transformation,
		depth,
		color,
		transformedColor))
	{
		CI_LOG_E("Failed to compute transformed color image\n");
		//return pointcloud; // most the time, if color is given, but cannot transformed, some more data is malicious
		noci::Color = true;
	}

	if (K4A_RESULT_SUCCEEDED != k4a_transformation_depth_image_to_point_cloud(m_transformation,
		depth,
		K4A_CALIBRATION_TYPE_DEPTH,
		pointcloudImage))
	{
		CI_LOG_E("Failed to compute point cloud\n");
		return pointcloud;
	}

	if (!pointcloudImage)
		return pointcloud;

	int16_t* pointcloudData = (int16_t*)(void*)k4a_image_get_buffer(pointcloudImage);
	uint8_t* colorData;
	if(!noci::Color && transformedci::Color != nullptr)
		colorData = k4a_image_get_buffer(transformedColor);

	if (!pointcloudData)
		return pointcloud;

	pointcloud->resize(static_cast<size_t>(height) * width);
	pointcloud->width = width;
	pointcloud->height = height;



	long i{ 0 };
	float min = 10000;
	float max = 0;

	for (auto& point : *pointcloud)
	{
		point.x = pointcloudData[3 * i + 0] * -0.001f;	// Convert to mm to m
		point.y = pointcloudData[3 * i + 1] * -0.001f;
		point.z = pointcloudData[3 * i + 2] * 0.001f;

		if (point.z < min)
			min = point.z;
		if (point.z > max)
			max = point.z;

		if (noColor) {
			point.r = 180.f;// 240.f - (point.z);
			point.g = 180.f;// 230.f;
			point.b = 180.f;// 230.f + (point.z);
		}
		else {
			point.b = colorData[4 * i + 0];
			point.g = colorData[4 * i + 1];
			point.r = colorData[4 * i + 2];
			// uint8_t alpha = color_image_data[4 * i + 3];
		}

		i = i + 1;
	}

	//TODO 

	return pointcloud;
}*/

void KinectDevice::startDevice(bool mode)
{
#ifdef WITHKINECT
	if (m_state == KinectState::KS_CLOSED) {
		openDevice();
	}

	try {
		m_device.start_cameras(&m_deviceConfiguration);
	}
	catch (std::exception exc) {
		CI_LOG_EXCEPTION("Failed to start Kinect cameras", exc);
	}

	try {
		m_device.start_imu();
	}
	catch(k4a::error err)
	{
		CI_LOG_E("Failed to start kinect IMU readings");
	}

	bodyTrackingInit(mode);
#endif;
	m_state = KinectState::KS_STARTUP;
}

void KinectDevice::stopDevice()
{
	if (m_state != KinectState::KS_CLOSED) {
#ifdef WITHKINECT
		m_tracker.shutdown();
		m_tracker.destroy();

		m_device.stop_cameras();
		m_device.stop_imu();

		m_device.close();
#endif;
		m_state = KinectState::KS_CLOSED;
	}
}

void KinectDevice::update() 
{
	if (m_state == KinectState::KS_CAPTURING)
	{
#ifdef WITHKINECT
		k4a::capture capture;
		if (m_device.get_capture(&capture, std::chrono::milliseconds(K4A_WAIT_INFINITE)))
		{
			k4a::image colorImage = capture.get_color_image();
			k4a::image depthImage = capture.get_depth_image();
			if (m_isCapturingImage)
				updateColor(colorImage);

			if (m_isCapturingDepth) {
				updateDepth(depthImage);
			}

			if (m_isProvidingPointCloud && depthImage != nullptr) {
				updatePointCloud(depthImage, colorImage);
			}

			/*
			if (colorImage != nullptr)
				colorImage.reset();
			if (depthImage != nullptr)
				depthImage.reset();
			*/

			if (m_isCapturingIR)
				updateIR(capture);

			if (m_isCapturingIMU)
				updateIMU();

			if (m_isCapturingBodies)
			{
				try {
					if (m_tracker.enqueue_capture(capture))
					{
						updateBodyTracker();
					}
				}
				catch(k4a::error err) {
					// It should never hit timeout when K4A_WAIT_INFINITE is set.
					CI_LOG_E("Error! Add capture to tracker process queue timeout!");
				}
			}
		}
#endif
	}
	else if (m_state == KinectState::KS_STARTUP) {
#ifdef WITHKINECT
		k4a::capture capture;
		try {
			if (m_device.get_capture(&capture, std::chrono::milliseconds(K4A_WAIT_INFINITE)))
			{
				k4a::image colorImage = capture.get_color_image();
				k4a::image depthImage = capture.get_depth_image();

			}

			m_state = KinectState::KS_CAPTURING;
		}
		catch (k4a::error err) {
			CI_LOG_E("Unable to get Capture");
		}
#endif
	}
}

glm::vec2 act::room::KinectDevice::getFOV()
{
	// TODO: add fancy function for getting FOV from kinect m_device
	return glm::vec2(90.0f, 74.3f); // in degree
}

void KinectDevice::updateColor(k4a::image colorImage)
{
#ifdef WITHKINECT
	if (colorImage != nullptr)
	{
		m_colorFrame = kh::k4a_get_mat(colorImage).getUMat(cv::ACCESS_FAST);

		if (m_colorFrame.rows != 0 && m_colorFrame.cols != 0) {
			if (m_isDrawing2dJoints)
				update2dJoints();

			cv::flip(m_colorFrame, m_colorFrame, 1);

			m_isColorFrameAvailable = true;
		}
	}
#endif
}

void KinectDevice::update2dJoints()
{
	if (m_skeletonJoints2d[1].size() == 32) {

		cv::UMat overlay;

		int circleSize = 20;
		cv::Scalar color = (255.0f, 0.0f, 0.0f, 255.0f);
		double alpha = 0.2;

		m_colorFrame.copyTo(overlay);

		for (int i = 1; i <= m_skeletonJoints2d.size(); i++) {
			for (int j = 0; j < 32; j++)
			{
				if (m_skeletonJoints2d[i][j].xy.x != -1.0f)
					cv::circle(overlay, cv::Point(m_skeletonJoints2d[i][j].xy.x, m_skeletonJoints2d[i][j].xy.y), circleSize, color, -1);
			}
		}
		cv::addWeighted(overlay, alpha, m_colorFrame, 1 - alpha, 0, m_colorFrame);
	}
}

void KinectDevice::updateDepth(k4a::image depthImage)
{
#ifdef WITHKINECT
	if (depthImage != nullptr)
	{
		/*
		int width	= depthImage.get_width_pixels();
		int height	= depthImage.get_height_pixels();

		if (width == 0 || height == 0)
			return;

		k4a::image transformedDepthImage;

		try {
			transformedDepthImage = k4a::image::create(K4A_IMAGE_FORMAT_DEPTH16, width, height, width * (int)sizeof(uint16_t));
		}
		catch(k4a::error err)
		{
			CI_LOG_E("Failed to create transformed depth image");
		}

		try {
			m_transformation.depth_image_to_color_camera(depthImage, &transformedDepthImage);
		}
		catch(k4a::error err)
		{
			transformedDepthImage = depthImage;
			CI_LOG_E("Failed to compute transformed depth image");
		}
		m_depthFrame = kh::k4a_get_mat(transformedDepthImage).getUMat(cv::ACCESS_FAST);
		*/
		m_depthFrame = kh::k4a_get_mat(depthImage).getUMat(cv::ACCESS_FAST);
		cv::flip(m_depthFrame, m_depthFrame, 1);
		m_isDepthFrameAvailable = true;
		
	}
#endif
}

void act::room::KinectDevice::updatePointCloud(k4a::image depth, k4a::image color)
{
	// m_pointcloud = toPointCloud(depth, color);
	// m_isPointCloudAvailable = true;
}

void KinectDevice::updateIR(k4a::capture capture)
{
#ifdef WITHKINECT
	k4a::image irImage = capture.get_ir_image();
	if (irImage.is_valid())
	{
		auto handle = irImage.handle();
		m_infraRedFrame = kh::k4a_get_mat(handle).getUMat(cv::ACCESS_FAST);
		cv::flip(m_infraRedFrame, m_infraRedFrame, 1);
		m_isInfraRedFrameAvailable = true;
	}
#endif
}

void KinectDevice::updateIMU()
{
#ifdef WITHKINECT
	k4a_imu_sample_t imuSample;

	if (m_device.get_imu_sample(&imuSample))
	{
		m_temperatureSample = imuSample.temperature;
		m_accelerometerSample = glm::vec3(imuSample.acc_sample.xyz.x, imuSample.acc_sample.xyz.y, imuSample.acc_sample.xyz.z);
		m_gyroscopeSample = glm::vec3(imuSample.gyro_sample.xyz.x, imuSample.gyro_sample.xyz.y, imuSample.gyro_sample.xyz.z);
	}
#endif
}

void KinectDevice::updateBodyTracker()
{
#ifdef WITHKINECT
	k4abt::frame bodyFrame = m_tracker.pop_result();

	if (bodyFrame != nullptr)
	{
		m_numOfBodies = bodyFrame.get_num_bodies();
		updateSkeletons(bodyFrame);
		updateBodyIndexMap(bodyFrame);
	}
	else
	{
		// CI_LOG_D("Pop body frame result failed!");
		return;
	}
#endif
}

void KinectDevice::updateSkeletons(k4abt::frame bodyFrame)
{
#ifdef WITHKINECT
	m_bodyMap.clear();

	for (size_t i = 0; i < m_numOfBodies; i++)
	{
		k4abt_body_t body = bodyFrame.get_body(i);

		k4abt_skeleton_t skeleton = body.skeleton;

		uint32_t id = body.id;
		m_bodyMap[id] = skeleton;
		m_areBodiesAvailable = true;

		if (m_isDrawing2dJoints)
			generateSkeleton2dCoordinates(skeleton, id);
	}
#endif
}

void KinectDevice::updateBodyIndexMap(k4abt::frame bodyFrame)
{
#ifdef WITHKINECT
	k4a::image bodyIndexMap = bodyFrame.get_body_index_map();
	if (bodyIndexMap.is_valid())
	{
		m_bodyIndexFrame = kh::k4a_get_mat(bodyIndexMap).getUMat(cv::ACCESS_FAST); // .mul(32); // expand space
		cv::flip(m_bodyIndexFrame, m_bodyIndexFrame, 1);
		m_isBodyIndexFrameAvailable = true;
	}
#endif
}

// xcopy "$(SolutionDir)..\blocks\onnxruntime\lib\onnxruntime.dll" "$(SolutionDir)bin\$(Configuration)\onnxruntime.dll" /Y /I
void KinectDevice::bodyTrackingInit(bool cpuMode)
{
#ifdef WITHKINECT
	k4abt_tracker_configuration_t trackerConfig = K4ABT_TRACKER_CONFIG_DEFAULT;
	if(cv::cuda::getCudaEnabledDeviceCount() > 0)
		trackerConfig.processing_mode = K4ABT_TRACKER_PROCESSING_MODE_GPU_CUDA;

	if (cpuMode)
	{
		trackerConfig.processing_mode = k4abt_tracker_processing_mode_t::K4ABT_TRACKER_PROCESSING_MODE_CPU;
	}
	try {
		m_tracker = k4abt::tracker::create(m_sensorCalibration);
	}
	catch (k4a::error err) {
		CI_LOG_E("Failed to create Tracker.");
	}
#endif
}

void KinectDevice::generateSkeleton2dCoordinates(k4abt_skeleton_t skeleton, int id)
{
	std::map<uint32_t, k4a_float2_t > jointMap;
#ifdef WITHKINECT
	for (int i = 0; i < (int)K4ABT_JOINT_COUNT; i++)
	{
		jointMap.insert(std::pair<uint32_t, k4a_float2_t>(i, convert3dTo2d(skeleton.joints[i].position)));
	}
	m_skeletonJoints2d[id] = jointMap;
#endif
}

k4a_float2_t KinectDevice::convert3dTo2d(k4a_float3_t point)
{
#ifdef WITHKINECT
	k4a_float2_t targetPoint;

	int result;

	k4a_calibration_3d_to_2d(&m_sensorCalibration, &point, K4A_CALIBRATION_TYPE_DEPTH, K4A_CALIBRATION_TYPE_COLOR, &targetPoint, &result);

	if (result != 1)
	{
		targetPoint.xy.x = -1.0f;
		targetPoint.xy.y = -1.0f;
	}

	return targetPoint;
#endif
};

void KinectDevice::getSerialNumber()
{
#ifdef WITHKINECT
	try {
		this->m_name = m_device.get_serialnum();
	}
	catch (k4a::error err) {
		CI_LOG_E(m_deviceIndex << ": Failed to get serial number");
		m_device.close();
		m_device = NULL;
	}
#endif
	//this->m_name = "";
}