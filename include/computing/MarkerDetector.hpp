
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

#include "DetectorBase.hpp"

#include "camera/CameraManager.hpp"
#include "marker/MarkerRoomNode.hpp"
#include "camera/CameraRoomNode.hpp"
#if __has_include(<opencv2/aruco.hpp>)
#define WITHARUCO
#endif

#ifdef WITHARUCO
#include <opencv2/aruco.hpp>
#endif


namespace act {
	namespace comp {

		struct MarkerOccurence {
			MarkerOccurence(int id) {
				this->id = id;
			}
			static std::shared_ptr<MarkerOccurence> create(int id) {return std::make_shared< MarkerOccurence>(id); }
			int id;
			int occurence = 0;
			bool wasInLastFrame = true;
		};

		struct MarkerCandidate {
			int id;
			cv::Vec3d tvec;
			cv::Vec3d rvec;
		};

		class MarkerDetector : public DetectorBase<MarkerCandidate>{
		public:
			MarkerDetector();
			MarkerDetector(room::CameraRoomNodeRef camera);
			~MarkerDetector();

			static	std::shared_ptr<MarkerDetector> create(room::CameraRoomNodeRef camera) { return std::make_shared<MarkerDetector>(camera); };

			virtual ci::Json toJson() override;
			virtual void fromJson(ci::Json json) override;

			
			glm::vec3 transformPosition(cv::Vec3d tvec, glm::vec3 parentPosition, glm::quat parentOrientation);
			glm::mat4 transformRotation(cv::Vec3d tvec, cv::Vec3d rvec, glm::vec3 parentPosition, glm::quat parentOrientation);
			void setCameraInverseToMarker(cv::Vec3d rvec, cv::Vec3d tvec, glm::vec3 markerPosition, glm::quat markerOrientierung);

			void setMarkerSize(float size) { m_markerSize = size; refreshObjPoints(); }

		private:
			void refreshObjPoints();
			float m_markerSize = 0.18f;

			cv::Mat m_objPoints;
			bool m_detecting = true;
			void detect() override;

			void checkOccurency(int id);

			std::vector<cv::Vec3d> inversePerspective(cv::Vec3d rvec, cv::Vec3d tvec);

			cv::aruco::Dictionary m_markerDictionary;
			cv::aruco::DetectorParameters m_detectorParameters;
			cv::aruco::ArucoDetector m_detector;

		 glm::ivec2 m_displaySize = glm::ivec2(640, 360);

			std::map<int, bool> m_validMarkerIDs;
			std::map<int, std::shared_ptr<MarkerOccurence>> m_newMarkerOccurence;


			std::thread m_thread;
		};
		using MarkerDetectorRef = std::shared_ptr<MarkerDetector>;
	}
}
