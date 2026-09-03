
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

		class ObjectRoomNode : public RoomNodeBase
		{
		public:
			ObjectRoomNode(int id, std::string name, act::UID replyUID = "");
			virtual ~ObjectRoomNode();
			static std::shared_ptr<ObjectRoomNode> create(int id, std::string name, act::UID replyUID = "") { return std::make_shared<ObjectRoomNode>(id, name, replyUID); };
			
			virtual void setup()	override;
			virtual void update()	override;
			virtual void draw()		override;

			virtual void drawSpecificSettings() override;

			virtual ci::Json toParams() override;
			virtual void fromParams(ci::Json json) override;
			
			bool isDynamic() { return m_dynamic; }
			int getID() { return m_id; }
		
		private:
			ci::gl::TextureRef m_objectTexture;
			float	m_objectWidth = 0.5f;
			bool	m_dynamic = true;
			int		m_id = -1;
			
			void setObjectImage();

		}; using ObjectRoomNodeRef = std::shared_ptr<ObjectRoomNode>;
	}
}
