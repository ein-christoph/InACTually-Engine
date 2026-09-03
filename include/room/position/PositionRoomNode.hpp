
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
#include "cinder/BSpline.h"


namespace act {
	namespace room {

		class PositionRoomNode : public RoomNodeBase
		{
		public:
			PositionRoomNode(std::string name, glm::vec3 position, act::UID replyUID = "");
			virtual ~PositionRoomNode();

			static std::shared_ptr<PositionRoomNode> create(std::string name, glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), act::UID replyUID = "") { return std::make_shared<PositionRoomNode>(name, position, replyUID); };


			virtual void setup()	override;
			virtual void update()	override;
			virtual void draw()		override;

			virtual void drawSpecificSettings() override;

			virtual ci::Json toParams() override;
			virtual void fromParams(ci::Json json) override;

			int			addControlPoint(glm::vec3 position); // return index;
			void		removeControlPoint(int index);
			glm::vec3	getControlPoint(int index);
			glm::vec3	getLastControlPoint();
			void		setControlPoint(int index, glm::vec3 position);
			std::vector<glm::vec3> getControlPoints() { return m_controlPoints; };
			int			getIndexOfControlPointAt(glm::vec3 position);

			void		setIsLooping(bool isLooping);
			bool		getIsLooping() { return m_isLooping; };
			int			setDegree(int degree);
			int			getDegree() { return m_degree; };

			glm::vec3	evaluatePosition(float t = 0.0f);

		private:
			std::vector<glm::vec3>	m_controlPoints;
			float					m_t;
			ci::BSpline3f			m_spline;
			std::vector<glm::vec3>	m_points;
			int						m_degree;
			void					updateSpline();

			glm::vec3				m_evaluatedPosition;
			bool					m_isLooping = false;

			bool					m_isHighlighted = false;


		}; using PositionRoomNodeRef = std::shared_ptr<PositionRoomNode>;
		
	}
}