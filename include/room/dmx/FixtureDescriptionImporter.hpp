/*
	InACTually
	> interactive theater for actual acts
	> this file is part of the "InACTually Engine", a MediaServer for driving all technology

	Copyright (c) 2026 InACTually Community
	Licensed under the MIT License.
	See LICENSE file in the project root for full license information.

	This file is created and substantially modified: 2026

	contributors:
	ein-christoph
*/

#pragma once

#include "roompch.hpp"
#include "dmx/OFLDescriptionMapper.hpp"

namespace act {
	namespace room {
		class FixtureDescriptionImporter
		{
		public:
			FixtureDescriptionImporter(std::function<void(ci::Json)> importCallback);
			~FixtureDescriptionImporter();

			static std::shared_ptr<FixtureDescriptionImporter> create(std::function<void(ci::Json)> importCallback) { return std::make_shared<FixtureDescriptionImporter>(importCallback); };

			void	draw();
			void	update();
			void	drawOFLImport();

			bool m_showImporter;
		private:
			bool m_openOFLInBrowser = false;
			bool m_searchOFLAgain = false;
			bool m_isOFLListFilteres = false;
			bool m_oflFixtureFilterChanged = false;
			char m_oflFixtureFilterBuffer[128] = "";

			std::function<void(ci::Json)> m_importCallback;

			void filterOFLFixtures();
			OFLDescriptionMapperRef m_oflDescriptionMapper;

		}; using FixtureDescriptionImporterRef = std::shared_ptr<FixtureDescriptionImporter>;
	}
}