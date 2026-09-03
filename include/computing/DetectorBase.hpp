
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

#include "RoomNodeManagerBase.hpp"

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

		template <class C>
		class DetectorBase {
		public:
			DetectorBase(std::string name, room::CameraRoomNodeRef camera = nullptr)
				: m_name(name)
			{
				m_cameraImageInPort = proc::ImageInputPort::create(proc::PT_IMAGE, "cameraImage", [&](cv::UMat image) {
					if (image.empty())
						return;

					m_currentImage = image;
					m_hasNewImage = true;
					});

				if (camera)
					setCamera(camera);
			}

			~DetectorBase() {
				doDetecting(false);
			};

			virtual ci::Json toJson() { return ci::Json::object(); };
			virtual void fromJson(ci::Json json) {};

			std::string getName() { return m_name; }

			room::CameraRoomNodeRef getCamera() { return m_camera; };

			void setCamera(room::CameraRoomNodeRef camera, bool isDetecting = true) {
				if (camera && m_camera && camera->getUID() == m_camera->getUID()) {
					doDetecting(isDetecting);
					return;
				}

				if (m_camera)
					m_camera->getCameraImagePort()->disconnect(m_cameraImageInPort);

				m_camera = camera;

				if (camera) {
					m_camera->getCameraImagePort()->connect(m_cameraImageInPort);
					doDetecting(isDetecting);
				}
				else {
					doDetecting(false);
				}
			}

			cv::UMat getFeedbackImage() { return m_feedbackImage; }

			ci::gl::TextureRef getFeedbackTexture() {
				cv::UMat mat = cv::UMat(m_feedbackImage);
				//m_feedbackImage.copyTo(mat);
				auto img = ci::fromOcv(mat);
				if (img)
					return ci::gl::Texture::create(img, ci::gl::Texture::Format().loadTopDown());

				return nullptr;
			}


			void doDetecting(bool detecting = true) {
				if (m_detecting == detecting)
					return;

				m_detecting = detecting;

				if (m_detecting) {
					m_thread = std::thread([&]() {
						while (m_detecting) {
							if (!m_hasNewImage || !m_isInitialized)
								continue;

							m_hasNewImage = false;
							detect();
						}
						});
				}
				else {
					while (!m_thread.joinable()) {
						std::this_thread::sleep_for(5ms);
					}
					m_thread.join();
				}
			}

			bool isDetecting() { return m_detecting; }

			bool hasNewCandidates() { return m_areNewCandidatesAvailable; }
			std::vector<C> getCandidates() { m_areNewCandidatesAvailable = false; return m_currentCandidates; }

		protected:
			bool m_isInitialized = false;

			cv::UMat m_currentImage;
			cv::UMat m_feedbackImage;
			room::CameraRoomNodeRef m_camera;

			std::vector<C> m_currentCandidates;
			bool m_areNewCandidatesAvailable = false;

			virtual void detect() = 0;

		private:
			std::string m_name;

			proc::ImageInputPortRef m_cameraImageInPort;
			bool m_hasNewImage = false;

			bool m_detecting = false;
			std::thread m_thread;

		};
	}
}
