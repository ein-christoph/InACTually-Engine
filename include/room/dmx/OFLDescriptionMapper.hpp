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

namespace act {
	namespace room {
		class OFLDescriptionMapper
		{
		public:
			OFLDescriptionMapper();
			~OFLDescriptionMapper();

			static std::shared_ptr<OFLDescriptionMapper> create() { return std::make_shared<OFLDescriptionMapper>(); };

			struct InternalParameterInfo {
				std::string internalParamName;
				bool hasNote = false;
			};

			struct OFLMode {
				std::string name;
				std::vector<std::string> channels;
				std::vector<InternalParameterInfo> internalParamsMapping; // cache for the translation of dmx channel to internal parameter name
				ci::Json internalDescription;
			}; using OFLModeRef = std::shared_ptr<OFLMode>;

			struct OFLFixtureDescription {
				std::string name;
				fs::path descriptionPath;
				ci::Json externalDescription;
				std::vector<OFLModeRef> modes;
				std::string type;
				int selectedMode;
				bool isDescriptionLoaded; // externalDescription loaded and metadata (name, modes, type) set
				bool showInListing = true; // used for filtering in UI
			}; using OFLFixtureDescriptionRef = std::shared_ptr<OFLFixtureDescription>;

			struct OFLManufacturer {
				std::string key;
				std::string name;
				std::vector<OFLFixtureDescriptionRef> fixtures;
				bool showInListing = true; // used for filtering UI
				bool expandInListing = false; // used to automatically and show fixtures in UI
			}; using OFLManufacturerRef = std::shared_ptr<OFLManufacturer>;

			// The OpenFixtureLibaray struct holds a vector of manufactueres
			// which each hold a vector of fixtures
			// each holding a vector of modes
			struct OpenFixtureLibaray {
				std::string name;
				fs::path libraryPath;
				std::vector<OFLManufacturerRef> manufacturers{};
				bool isParsed;
			}; using OpenFixtureLibarayRef = std::shared_ptr<OpenFixtureLibaray>;


			std::string getName();
			bool searchLibraryPath();
			std::string getLibarayPath();
			bool setLibarayPath(std::string);
			std::vector<OFLManufacturerRef> getManufacturers(bool allowParsing);

			bool parseLibraryMeta();

			bool parseBasicDescription(OFLFixtureDescriptionRef fixtureDescription);

			bool translateOFLtoInternal(OFLFixtureDescriptionRef fixtureDescription);

		private:
			OpenFixtureLibaray m_ofl;

			std::vector<InternalParameterInfo> getInternalParameterMapping(ci::Json internalDescription);
			int convertDegStrToInt(std::string degStr, std::string decimalpoint = ".");

			// Checks if array of objects contains the key with the value or just the value / key if one is empty
			// returns index or -1 if nothing is found
			int isInJsonObjArray(std::string key, std::string value, ci::Json array);


			// Takes fineChannelAliases array and resolves them to dmx Offset in the channels List of the given mode
			// Returns map of fine channel alias with corresponding int or -1 if fineChannelAlias could not be resolved
			std::map<std::string, int> resolveFineChannels(ci::Json const& fullExtDesc, int mode, ci::Json const& extChannelDesc, int maxAliases = 1);

			/* Translates a Capability of a Channel description into a patch of the internal description calling the corresponding translate methods of the capability
			* (   ci::Json const& internalDesc		current internal description to which the returned patch can be applied
			    , ci::Json const& extCapabilityDesc	reference to the capability which should be translated
				, ci::Json const& extChannelDesc	reference to the full channel description in which the capability to translate is used
				, ci::Json const& fullExtDesc		reference to the full external description if other information is needed
				, int dmxOffset						dmx offset = key of the channel in the channels list of the mode
				, int modeIndex						key of the mode to translate
			  )
			*/
			ci::Json translateChannelCapability(ci::Json const& internalDesc, ci::Json const& extCapabilityDesc, ci::Json const& extChannelDesc, ci::Json const& fullExtDesc, int dmxOffset, int modeIndex);

			// Handles translation of Channels with multiple capabilities
			ci::Json translateCapabilitiesChannel(ci::Json const& internalDesc, ci::Json const& extCapabilitiesDesc, ci::Json const& extChannelDesc, ci::Json const& fullExtDesc, int dmxOffset, int modeIndex, std::string const& channelName);

			// NOTE: Translate methods of the capabilities shold all use the same signature to provide fast access to relevant context information.

			ci::Json translateIntensityCapability(ci::Json const& internalDesc, ci::Json const& extCapabilityDesc, ci::Json const& extChannelDesc, ci::Json const& fullExtDesc, int dmxOffset, int modeIndex);
			ci::Json translatePanCapability(ci::Json const& internalDesc, ci::Json const& extCapabilityDesc, ci::Json const& extChannelDesc, ci::Json const& fullExtDesc, int dmxOffset, int modeIndex);
			ci::Json translateTiltCapability(ci::Json const& internalDesc, ci::Json const& extCapabilityDesc, ci::Json const& extChannelDesc, ci::Json const& fullExtDesc, int dmxOffset, int modeIndex);
			ci::Json translateZoomCapability(ci::Json const& internalDesc, ci::Json const& extCapabilityDesc, ci::Json const& extChannelDesc, ci::Json const& fullExtDesc, int dmxOffset, int modeIndex);
			ci::Json translateColorIntensityCapability(ci::Json const& internalDesc, ci::Json const& extCapabilityDesc, ci::Json const& extChannelDesc, ci::Json const& fullExtDesc, int dmxOffset, int modeIndex);
			ci::Json translatePanTiltSpeedCapability(ci::Json const& internalDesc, ci::Json const& extCapabilityDesc, ci::Json const& extChannelDesc, ci::Json const& fullExtDesc, int dmxOffset, int modeIndex);
			
			bool isColorWheel(std::string const& channelName, ci::Json const& extChannelDesc, ci::Json const& fullExtDesc, int dmxOffset, int modeIndex);
			ci::Json translateColorWheelCapability(ci::Json const& internalDesc, ci::Json const& extCapabilityDesc, ci::Json const& extChannelDesc, ci::Json const& fullExtDesc, int dmxOffset, int modeIndex, std::string const& channelName);

		}; using OFLDescriptionMapperRef = std::shared_ptr<OFLDescriptionMapper>;
	}
}