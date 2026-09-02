
/*
	InACTually
	> interactive theater for actual acts
	> this file is part of the "InACTually Engine", a MediaServer for driving all technology

	Copyright (c) 2026 InACTually Community
	Licensed under the MIT License.
	See LICENSE file in the project root for full license information.

	This file is created and substantially modified: 2026

	contributors:
	Lars Engeln - mail@lars-engeln.de
*/

#pragma once

#include "oneapi/tbb.h"
#include "oneapi/tbb/flow_graph.h"

namespace act {
	namespace proc {
		
		class FlowRuntime
		{
		public:
			static tbb::flow::graph& getGraph()
			{
				static tbb::flow::graph instance;
				return instance;
			}

			static void wait()
			{
				getGraph().wait_for_all();
			}

		private:
			FlowRuntime() = delete;
		};
		
		using FlowRuntimeRef = std::shared_ptr<FlowRuntime>;
	}
}