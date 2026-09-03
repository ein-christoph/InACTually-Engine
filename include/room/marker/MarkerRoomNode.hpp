
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

#include "RoomNodeBase.hpp"


namespace act {
	namespace room {

		class MarkerRoomNode : public RoomNodeBase
		{
		public:
			MarkerRoomNode(int id, act::UID replyUID = "");
			virtual ~MarkerRoomNode();
			static std::shared_ptr<MarkerRoomNode> create(int id, act::UID replyUID = "") { return std::make_shared<MarkerRoomNode>(id, replyUID); };
			
			virtual void setup()	override;
			virtual void update()	override;
			virtual void draw()		override;
			
			int getID() { return m_id; }
			bool isDynamic() { return m_dynamic; }

			virtual void drawSpecificSettings() override;

			virtual ci::Json toParams() override;
			virtual void fromParams(ci::Json json) override;

		private:
			ci::gl::TextureRef m_markerTexture;
			bool	m_dynamic = true;
			int		m_id;
			float	m_markerWidth = 0.18f;
			
			void setMarkerImage();

		}; using MarkerRoomNodeRef = std::shared_ptr<MarkerRoomNode>;
	}
}
