
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

#pragma once

#include "roompch.hpp"
#include "KinectHelper.hpp"

#if __has_include(<k4a/k4a.h>)
#define WITHKINECT
#endif

#ifdef WITHKINECT
//#include <k4a/k4a.h>
#include <k4a/k4a.hpp>
#include <k4abt.hpp>
#else
using k4a_float3_t = struct {
	struct xyz {
		float x;
		float y;
		float Z;
	} xyz;
};
using k4a_quaternion_t = struct {
	struct wxyz {
		float w;
		float x;
		float y;
		float Z;
	} wxyz;
};
using k4abt_joint_confidence_level_t = int;
using k4abt_joint_t = struct {
	k4abt_joint_confidence_level_t confidence_level;
	k4a_float3_t position;
	k4a_quaternion_t orientation;
};

using k4a_device_t = std::string;
using k4abt_skeleton_t = struct k4a_Skeleton {
	int id;
	k4abt_joint_t joints[32];
};
using k4abt_frame_t = cv::UMat;
using k4a::image = cv::UMat;
using k4a_capture_t = std::string;
using k4a_device_configuration_t = std::string;
using k4a_calibration_t = std::string;
using k4abt_tracker_t = bool;

#endif


namespace act {
	namespace room {

		enum KinectState {
			KS_OPENED,
			KS_CLOSED,
			KS_STARTUP,
			KS_CAPTURING,
		};

		class KinectDevice {
		public:

			KinectDevice() : m_deviceIndex(-1), m_nodeUID("") {};
			KinectDevice(std::uint32_t deviceIndex);
			~KinectDevice();

			static std::shared_ptr<KinectDevice> create(std::uint32_t deviceIndex) { return std::make_shared<KinectDevice>(deviceIndex); };

			virtual void openDevice();
			virtual void startDevice(bool mode);
			virtual void stopDevice();
			
			virtual void update();

			k4a::device& getDevice() { return m_device; };
			std::string getName() { return m_name; };
			void setNodeUID(act::UID nodeUID) { m_nodeUID = nodeUID; };
			act::UID getNodeUID() { return m_nodeUID; };

			void setIsCapturingImage(bool isCapturing) { m_isCapturingImage = isCapturing; };
			void setIsCapturingDepth(bool isCapturing) { m_isCapturingDepth = isCapturing; };
			void setIsCapturingIR(bool isCapturing) { m_isCapturingIR = isCapturing; };
			void setIsCapturingIMU(bool isCapturing) { m_isCapturingIMU = isCapturing; };
			void setIsCapturingBodies(bool isCapturing) { m_isCapturingBodies = isCapturing; };

			void setIsProvidingPointCloud(bool isProviding) { m_isProvidingPointCloud = isProviding; };


			void setIsDrawing2dJoints(bool isDrawing2dJoints) { m_isDrawing2dJoints = isDrawing2dJoints; };

			void setRepositionedBodies(std::map<uint32_t, k4abt_skeleton_t> bodies) { m_repositionedBodyMap = bodies; };

			void setColorMap(cv::Mat colorMap) { m_colorMap = colorMap; };

			glm::vec2 getFOV();

			cv::UMat getColorFrame() { return m_colorFrame; };
			cv::UMat getDepthFrame() { return m_depthFrame; };
			cv::UMat getInfraRedFrame() { return m_infraRedFrame; };
			cv::UMat getBIMFrame() { return m_bodyIndexFrame; };
			//Pointcloud getPointCloud() { return m_pointcloud; };

			float getTemperatureSample() { return m_temperatureSample; };
			glm::vec3 getAccelerometerSample() { return m_accelerometerSample; };
			glm::vec3 getGyroscopeSample() { return m_gyroscopeSample; };


			std::map<uint32_t, k4abt_skeleton_t> getBodyMap() { return m_bodyMap; };
			std::map<uint32_t, k4abt_skeleton_t> getRepositionedBodies() { return m_repositionedBodyMap; };

			cv::Mat getColorMap() { return m_colorMap; };

			int getNumOfBodies() { return m_numOfBodies; };

			KinectState getCurrentState() { return m_state; };

			virtual bool isTracking() { return m_tracker != NULL; };
			bool isDummy() { return m_isDummy; };

			bool m_isColorFrameAvailable;
			bool m_isDepthFrameAvailable;
			bool m_isInfraRedFrameAvailable;
			bool m_isBodyIndexFrameAvailable;
			bool m_areBodiesAvailable;
			bool m_isPointCloudAvailable;

			k4a_float2_t convert3dTo2d(k4a_float3_t point);

		protected:
			std::string m_name;
			act::UID m_nodeUID;

			k4a::device m_device = nullptr;
			k4a::capture m_capture;
			k4a_device_configuration_t m_deviceConfiguration;

			k4a::calibration m_sensorCalibration;
			k4a::transformation m_transformation = NULL;
			k4abt::tracker m_tracker = nullptr;

			const uint32_t m_deviceIndex;

			cv::UMat m_colorFrame;
			cv::UMat m_depthFrame;
			cv::UMat m_infraRedFrame;
			cv::UMat m_bodyIndexFrame;
			// Pointcloud m_pointcloud;

			float m_temperatureSample;
			glm::vec3 m_accelerometerSample;
			glm::vec3 m_gyroscopeSample;

			std::map<uint32_t, k4abt_skeleton_t> m_bodyMap;
			std::map<uint32_t, k4abt_skeleton_t> m_repositionedBodyMap;
			int m_numOfBodies = -1;

			std::map<uint32_t, std::map<uint32_t, k4a_float2_t >> m_skeletonJoints2d;

			cv::Mat m_colorMap;

			//Pointcloud toPointCloud(k4a::image depth, k4a::image color = nullptr);

			KinectState m_state = KinectState::KS_CLOSED;

			bool m_isCapturingBodies = true;
			bool m_isCapturingImage = false;
			bool m_isCapturingDepth = false;
			bool m_isCapturingIR = false;
			bool m_isCapturingIMU = false;

			bool m_isProvidingPointCloud = true;

			bool m_isDrawing2dJoints = false;

			bool m_isDummy = false;


			void updateColor(k4a::image colorImage);
			void updateDepth(k4a::image depthImage);
			void updatePointCloud(k4a::image depth, k4a::image color = nullptr);
			void updateIR(k4a::capture capture);

			void updateIMU();


		private:


			void initialize();
			void bodyTrackingInit(bool cpuMode);

			void getSerialNumber();

			void update2dJoints();

			void updateBodyTracker();
			void updateSkeletons(k4abt::frame bodyFrame);
			void updateBodyIndexMap(k4abt::frame bodyFrame);

			void generateSkeleton2dCoordinates(k4abt_skeleton_t skeleton, int id);
		};
		using KinectDeviceRef = std::shared_ptr<KinectDevice>;
	}
}