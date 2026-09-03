
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

#include <memory>
#include "cinder/Json.h"
#include <functional>

#include "Port.hpp"


namespace act {
	namespace net {

		class RPC {
		public:
			RPC(std::string functionName, std::function<bool(void)> recieveCallback)
				: m_functionName(functionName), m_recieveCallback(recieveCallback) {};

			static std::shared_ptr<RPC> create(std::string functionName, std::function<bool(void)> recieveCallback) { return std::make_shared<RPC>(functionName, recieveCallback); };

			virtual bool call() { return m_recieveCallback(); };

			proc::InputPortRef<bool> createPort(std::string label = "") {
				if (label == "")
					label = m_functionName;
				return proc::InputPort<bool>::create(proc::PT_BOOL, label, [&](bool value = true) {
					if (value) {
						m_recieveCallback();
					}
				});
			};

		private:
			std::string					m_functionName;
			std::function<bool(void)>	m_recieveCallback;
		}; using RPCRef = std::shared_ptr<RPC>;

		class RPCHandler {
		public:
			RPCHandler() {};

			RPCRef addRPC(std::string functionName, std::function<bool(void)> recieveCallback) {
				m_rpcs[functionName] = RPC::create(functionName, recieveCallback);
				return m_rpcs[functionName];
			}
			RPCRef addRPC(proc::InputPortRef<bool> inputPort) {
				m_rpcs[inputPort->getName()] = RPC::create(inputPort->getName(), [inputPort]() {
					inputPort->recieve(true); 
					return true;
				});
				return m_rpcs[inputPort->getName()];
			}

			bool call(std::string functionName) {
				if (!m_rpcs.contains(functionName))
					return false;

				return m_rpcs[functionName]->call();
			};

			std::map<std::string, RPCRef> getRPCMap() { return m_rpcs; };

		private:
			std::map<std::string, RPCRef> m_rpcs;

		}; using RPCHandlerRef = std::shared_ptr<RPCHandler>;
	}
}