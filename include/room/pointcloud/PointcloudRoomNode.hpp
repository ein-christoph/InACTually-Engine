
/*
	InACTually
	> interactive theater for actual acts
	> this file is part of the "InACTually Engine", a MediaServer for driving all technology

	Copyright (c) 2021–2025 Lars Engeln, Fabian Töpfer
	Copyright (c) 2025 InACTually Community
	Licensed under the MIT License.
	See LICENSE file in the project root for full license information.

	This file is created and substantially modified: 2023-2024

	contributors:
	Lars Engeln - mail@lars-engeln.de
*/

#pragma once

#include <memory>

#include "RoomNodeBase.hpp"


namespace act {
	namespace room {

		using Pointcloud = std::shared_ptr<std::vector<glm::vec3>>;

		class PointcloudRoomNode : public RoomNodeBase {
		public:
			PointcloudRoomNode(glm::vec3 position, float radius, std::string name = "", act::UID replyUID = "");
			~PointcloudRoomNode();

			static std::shared_ptr<PointcloudRoomNode> create(glm::vec3 position, float radius, std::string name = "", act::UID replyUID = "") { return std::make_shared<PointcloudRoomNode>(position, radius, name, replyUID); };

			std::string		getName() { return m_caption; };

			virtual void	setup()		override;
			virtual void	update()	override;
			virtual void	draw()		override;

			void setPointcloud(Pointcloud pointcloud);

			

		private:

			Pointcloud m_pointcloud;
			int m_numPoints;

			void createPointCloud();
			void updatePointCloud();
			std::vector<glm::vec3>		m_positions;
			std::vector<ci::Colorf> m_colors;
			std::vector<float>		m_sizes;
			ci::gl::BatchRef		m_geometry;
			ci::gl::GlslProgRef		m_shader;

			void drawSpecificSettings() override;
		};
		using PointcloudRoomNodeRef = std::shared_ptr<PointcloudRoomNode>;
	}
}