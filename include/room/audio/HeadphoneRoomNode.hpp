
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

#include "audio/SpeakerRoomNode.hpp"
#include "cinder/audio/GenNode.h"
#include "cinder/audio/audio.h"
#include "cinder/Timeline.h"

#include <memory>


namespace act {
	namespace room {

		class HeadphoneRoomNode : public SpeakerRoomNode {
		public:
			HeadphoneRoomNode(int channelL, int channelR, glm::vec3 position, float radius);
			~HeadphoneRoomNode();

			static std::shared_ptr<HeadphoneRoomNode> create(int channelL, int channelR, glm::vec3 position, float radius) { return std::make_shared<HeadphoneRoomNode>(channelL, channelR, position, radius); };

			virtual void		setup()		override;
			virtual void		update()	override;
			virtual void		draw()		override;

			int					getChannel() { return m_leftSpeaker->getChannel(); }

			ci::audio::NodeRef	getIn()		override;
			ci::audio::NodeRef	getOut()	override;

			SpeakerRoomNodeRef	getL() { return m_leftSpeaker; };
			SpeakerRoomNodeRef	getR() { return m_rightSpeaker; };

			virtual void		setVolume(float volume, float rampDuration = 0.1f) override;

			virtual bool		hit(glm::vec3 pos) override;
			virtual bool		hitRay(ci::Ray ray) override; 

			virtual	void		setIsHovered(bool hovered)   override;
			virtual	void		setIsSelected(bool selected) override;

			void				disconnectExternals() override;
			void				setupAudioNodes();

			void				locate(bool isLocating = true);
			void				calibrate(std::string type, float duration, float volume);

			//void				setEQFilter(cv::UMat filterLeft, cv::UMat filterRight);
			//void				resetEQFilter();

		protected:
			SpeakerRoomNodeRef		m_leftSpeaker;
			SpeakerRoomNodeRef		m_rightSpeaker;

			ci::audio::ChannelRouterNodeRef m_channelRouterNodeLeft;
			ci::audio::ChannelRouterNodeRef m_channelRouterNodeRight;

			virtual void		drawSpecificSettings() override;
		};
		using HeadphoneRoomNodeRef = std::shared_ptr<HeadphoneRoomNode>;
	}
}