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

			struct OFLChannelDescPatch {
				ci::Json descriptionPatch;
				bool includePatch = true;
			}; using OFLChannelDescPatchRef = std::shared_ptr<OFLChannelDescPatch>;

			struct OFLChannelTranslation {
				std::string oflChannelKey;
				OFLChannelDescPatchRef channelDescPatch;
			}; using OFLChannelTranslationRef = std::shared_ptr<OFLChannelTranslation>;

			struct OFLMode {
				std::string name;
				std::map<int, OFLChannelTranslationRef> channelTranslationMapping; // cache for the translation of dmx channel to internal parameter name
				ci::Json internalDescBase;
			}; using OFLModeRef = std::shared_ptr<OFLMode>;

			struct OFLFixtureDescription {
				act::UID uid;
				std::string name;
				fs::path descriptionPath;
				ci::Json externalDescription;
				std::vector<OFLModeRef> modes;
				std::string type;
				int selectedMode;
				bool queuedForLoading = false;
				bool queuedForImport = false;
				bool hasError = false;
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
			fs::path getLibarayPath();
			bool setLibarayPath(fs::path path);
			bool getIsParsed();
			std::vector<OFLManufacturerRef> getManufacturers(bool allowParsing);

			bool parseLibraryMeta();

			bool parseBasicDescription(OFLFixtureDescriptionRef fixtureDescription);

			bool translateOFLtoInternal(OFLFixtureDescriptionRef fixtureDescription);

			static ci::Json getInternalDescription(OFLFixtureDescriptionRef fixture);

		private:
			std::string m_ManufacturerIdxFileName = "manufacturers.json";
			OpenFixtureLibaray m_ofl;

			void addDescToOtherAffectedChannels(OFLModeRef modeRef, OFLChannelDescPatchRef channelDescPatchRef, std::string const& channelKey, int primaryDmxOffset);
			void attachNoteToChannelDesc(ci::Json& internalDescPatch, int dmxOffset, std::string note);

			std::string getWheelNameKey(std::string const& channelName, ci::Json const& extCapabilities);
			int convertDegStrToInt(std::string degStr, std::string decimalpoint = ".");

			float speedToFloat(std::string speed);

			// Checks if array of objects contains the key with the value or just the value / key if one is empty
			// returns index or -1 if nothing is found
			int isInJsonObjArray(std::string key, std::string value, ci::Json array);

			// Calculates the distance in address offset from baseRange to distantRange
			// assumes lower address first, higher address second
			int dmxRangeDistance(ci::Json const& distantRange, ci::Json const& baseRange);
			int dmxRangeToDmxValue(ci::Json const& dmxRange);

			// Takes fineChannelAliases array and resolves them to dmx Offset in the channels List of the given mode
			// Returns map of fine channel alias with corresponding int or -1 if fineChannelAlias could not be resolved
			std::map<std::string, int> resolveFineChannels(ci::Json const& fullExtDesc, int mode, ci::Json const& extChannelDesc, int maxAliases = 1);

			//Translates a channel into a json patch of the internal description calling the corresponding translate methods
			ci::Json translateChannel(ci::Json const& extChannelDesc, ci::Json const& fullExtDesc, int dmxOffset, int modeIndex, std::string const& channelName);

			/* Translation methods should all use the same signature to provide fast access to relevant context information
			 * (  ci::Json const& extCapabilityDesc	reference to the capability which should be translated
				, ci::Json const& extChannelDesc	reference to the full channel description in which the capability to translate is used
				, ci::Json const& fullExtDesc		reference to the full external description if other information is needed
				, int dmxOffset						dmx offset = key of the channel in the channels list of the mode
				, int modeIndex						key of the mode to translate
				, std::string const& channelName	Optional, for translation of capabilities
			  )
			*/
			ci::Json translateIntensityCapability(ci::Json const& extCapabilityDesc, ci::Json const& extChannelDesc, ci::Json const& fullExtDesc, int dmxOffset, int modeIndex);
			ci::Json translatePanTiltCapability(ci::Json const& extCapabilityDesc, ci::Json const& extChannelDesc, ci::Json const& fullExtDesc, int dmxOffset, int modeIndex);
			ci::Json translateZoomCapability(ci::Json const& extCapabilityDesc, ci::Json const& extChannelDesc, ci::Json const& fullExtDesc, int dmxOffset, int modeIndex);
			ci::Json translateColorIntensityCapability(ci::Json const& extCapabilityDesc, ci::Json const& extChannelDesc, ci::Json const& fullExtDesc, int dmxOffset, int modeIndex);
			ci::Json translatePanTiltSpeedCapability(ci::Json const& extCapabilityDesc, ci::Json const& extChannelDesc, ci::Json const& fullExtDesc, int dmxOffset, int modeIndex);
			
			bool isColorWheel(std::string const& channelName, ci::Json const& extChannelDesc, ci::Json const& fullExtDesc, int dmxOffset, int modeIndex);
			ci::Json translateColorWheelCapability(ci::Json const& extCapabilityDesc, ci::Json const& extChannelDesc, ci::Json const& fullExtDesc, int dmxOffset, int modeIndex, std::string const& channelName);

			bool isGoboWheel(std::string const& channelName, ci::Json const& extChannelDesc, ci::Json const& fullExtDesc, int dmxOffset, int modeIndex);
			ci::Json translateGoboWheelCapability(ci::Json const& extCapabilityDesc, ci::Json const& extChannelDesc, ci::Json const& fullExtDesc, int dmxOffset, int modeIndex, std::string const& channelName);

			bool isShutterStrobeCapability(std::string const& channelName, ci::Json const& extChannelDesc, ci::Json const& fullExtDesc, int dmxOffset, int modeIndex);
			ci::Json translateShutterStrobeCapability(ci::Json const& extCapabilityDesc, ci::Json const& extChannelDesc, ci::Json const& fullExtDesc, int dmxOffset, int modeIndex, std::string const& channelName);

		}; using OFLDescriptionMapperRef = std::shared_ptr<OFLDescriptionMapper>;
	}
}