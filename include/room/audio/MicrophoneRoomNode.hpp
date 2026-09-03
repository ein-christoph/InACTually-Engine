
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

#include "audio/AudioRoomNodeBase.hpp"
#include "cinder/audio/GenNode.h"
#include "cinder/Timeline.h"

//#include "EqualizerNode.hpp"

#include <memory>


namespace act {
	namespace room {

		class MicrophoneRoomNode : public AudioRoomNodeBase {
		public:
			MicrophoneRoomNode(int channel, glm::vec3 position, float radius, std::string name);
			~MicrophoneRoomNode();

			static std::shared_ptr<MicrophoneRoomNode> create(int channel, glm::vec3 position, float radius, std::string name = "Mic") { return std::make_shared<MicrophoneRoomNode>(channel, position, radius, name); };

			virtual void		setup()		override;
			virtual void		update()	override;
			virtual void		draw()		override;

			int					getChannel() { return m_channel; }

			virtual ci::Json toParams();
			virtual void fromParams(ci::Json json);

			ci::audio::NodeRef	getIn()		override;
			ci::audio::NodeRef	getOut()	override;
			void				disconnectExternals() override;
			virtual void		setupAudioNodes();

			void calibrate(float duration, float volume);

			//void setEQFilter(cv::UMat eqFilter) { m_eq->setEQFilter(0, eqFilter); };
			//void resetEQFilter() { m_eq->bypass(); }

			virtual void drawSpecificSettings() override;

		protected:
			int m_channel;

			ci::audio::AddNodeRef		m_add;
			//EqualizerNodeRef			m_eq;

			ci::Anim<float>				m_calibrateProgress;

			
		};
		using MicrophoneRoomNodeRef = std::shared_ptr<MicrophoneRoomNode>;
	}
}