
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
	Fabian Töpfer
	Lars Engeln - mail@lars-engeln.de
*/

#pragma once

#include "ProcNodeBase.hpp"
 

namespace act {
	namespace proc {

		class LinkerProcNode : public ProcNodeBase
		{
		public:
			LinkerProcNode();
			~LinkerProcNode();
		
			PROCNODECREATE(LinkerProcNode);

			void update()				override;
			void draw()					override;
			void drawPorts(int width)	override;

			//ci::Json toParams() override;
		//	void fromParams(ci::Json json) override;


			void setOutputPort(PortBaseRef port) {
				m_outputPorts.resize(0);
				m_outputPorts.push_back(port);
			};

			void setInputPort(PortBaseRef port) {
				m_inputPorts.resize(0);
				m_inputPorts.push_back(port);
			};
 

			act::proc::PortBaseRef setInternalOutputPort(PortBaseRef toPort);
			act::proc::PortBaseRef setInternalInputPort(PortBaseRef fromPort);

			   PortBaseRef  getExternalInputPort() {
				  return m_externalInputPort;
			   }
			  PortBaseRef  getExternalOutputPort() {
				  return m_externalOutputPort;
			  }

			  PortBaseRef getInternalInputPort() {
				  return m_internalInputPort;
			  }
			  PortBaseRef getInternalOutputPort() {
				  return m_internalOutputPort;
			  }

			  PortBaseRef getInputPort() {
				  if (m_isInput) {
					  return m_externalInputPort;
				  }
				  return m_internalInputPort;
			  }
				
			  PortBaseRef getOutputPort() {
				  if (m_isInput) {
					  return m_internalOutputPort;
				  }
				  return m_externalOutputPort;
			  }

			  PortBaseRef getOutputPortByName(std::string name) override;
			  PortBaseRef getInputPortByName(std::string name) override;

			  ci::Json toParams() override;
			  void fromParams(ci::Json json) override;

			  bool getIsInput() { return m_isInput; };

			  PortBaseRef connectToPort(PortBaseRef toPort);
			  PortBaseRef connectFromPort(PortBaseRef fromPort);

		 
		private:

			bool m_isInput;


			//if used as input for group
			PortBaseRef m_externalInputPort;
			PortBaseRef m_internalOutputPort;


			//if used as output for group
			PortBaseRef m_externalOutputPort;
			PortBaseRef m_internalInputPort;
			

			void isInputChanged();
		};

		using LinkerNodeRef = std::shared_ptr<LinkerProcNode>;

	}
}