
/*
	InACTually
	> interactive theater for actual acts
	> this file is part of the "InACTually Engine", a MediaServer for driving all technology

	Copyright (c) 2021–2025 Lars Engeln, Fabian Töpfer
	Copyright (c) 2025 InACTually Community
	Licensed under the MIT License.
	See LICENSE file in the project root for full license information.

	This file is created and substantially modified: 2021-2025

	contributors:
	Lars Engeln - mail@lars-engeln.de
*/

#pragma once

#include "RoomNodeBase.hpp"


namespace act {
	namespace room {

		class RoomNodeManagerBase : public RoomNodeBase {
		public:
			RoomNodeManagerBase(std::string name);
			virtual ~RoomNodeManagerBase();

			//void	setup()		override;
			void	update()	override;
			void	draw()		override;
			void	cleanUp()	override;

			virtual act::room::RoomNodeBaseRef drawMenu() = 0;

			void				addNode(RoomNodeBaseRef node);
			RoomNodeBaseRef	getNodeByUID(act::UID uid);

			bool				hit(glm::vec3 pos)	override;
			bool				hitRay(ci::Ray ray) override;
			RoomNodeBaseRef	getNodeAtPos(glm::vec3 pos);
			RoomNodeBaseRef	getNodeOnRay(ci::Ray ray);

			bool	removeNode(act::UID uid);
			void	clear();

			const std::vector<RoomNodeBaseRef> getNodes() { return m_nodes; };

			virtual ci::Json toJson() = 0;
			virtual void fromJson(ci::Json json) = 0;

		protected:

			virtual void refreshLists() {};
			std::vector<RoomNodeBaseRef>	m_nodes;
		};
		using RoomNodeManagerBaseRef = std::shared_ptr<RoomNodeManagerBase>;
	}
}