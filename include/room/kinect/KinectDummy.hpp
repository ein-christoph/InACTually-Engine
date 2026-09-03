
/*
	InACTually
	> interactive theater for actual acts
	> this file is part of the "InACTually Engine", a MediaServer for driving all technology

	Copyright (c) 2021–2025 Lars Engeln, Fabian Töpfer
	Copyright (c) 2025 InACTually Community
	Licensed under the MIT License.
	See LICENSE file in the project root for full license information.

	This file is created and substantially modified: 2022

	contributors:
	Lars Engeln - mail@lars-engeln.de
*/

#pragma once

#include "cinder\app\App.h"
#include "CinderOpenCV.h"

#include "kinect/KinectDevice.hpp"
#include "VideoPlayerProcNode.hpp"

#include <k4arecord/playback.hpp>


namespace act {
	namespace room {

		class KinectDummy : public KinectDevice {
		public:

			KinectDummy(std::uint32_t deviceIndex, std::string name, std::string path, ci::Json bodyData);
			~KinectDummy();

			static std::shared_ptr<KinectDummy> create(std::uint32_t deviceIndex, std::string name, std::string path, ci::Json bodyData) { return std::make_shared<KinectDummy>(deviceIndex, name, path, bodyData); };

			void openDevice() override;
			void startDevice(bool mode) override;
			void stopDevice() override;

			void update() override;

			k4a::device& getDevice()				{ return m_device; };
			std::string getName()				{ return m_name; };
			uint32_t getDeviceIndex()				{ return m_deviceIndex; };
			
 
			bool isTracking() override { return true; };

		private:
			k4a_playback_t					m_playback;
			std::string						m_path;

			//int							m_currentBodyMapIndex;
			//std::vector<std::map<uint32_t, k4abt_skeleton_t>> m_bodyMaps;

			//void processBodyData(ci::Json bodyData);
		};
		using KinectDummyRef = std::shared_ptr<KinectDummy>;
	}
}