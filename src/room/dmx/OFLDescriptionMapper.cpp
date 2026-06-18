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

#include "roompch.hpp"
#include "dmx/OFLDescriptionMapper.hpp"
#include "utils/RGBAWHelper.h"

act::room::OFLDescriptionMapper::OFLDescriptionMapper()
{
	m_ofl.name = "Open Fixture Library";
	m_ofl.isParsed = false;
	searchLibraryPath();
}

act::room::OFLDescriptionMapper::~OFLDescriptionMapper()
{
}

std::string act::room::OFLDescriptionMapper::getName()
{
	return m_ofl.name;
}

bool act::room::OFLDescriptionMapper::searchLibraryPath()
{
	fs::path usualPath = app::getAssetPath("dmx/ofl_export_ofl");
	return setLibarayPath(usualPath);
}

fs::path act::room::OFLDescriptionMapper::getLibarayPath()
{
	return m_ofl.libraryPath;
}

bool act::room::OFLDescriptionMapper::setLibarayPath(fs::path path)
{
	// Check if a manufacturer index exists at provided path
	fs::path check = path;
	check.append(m_ManufacturerIdxFileName);
	if (!fs::exists(check))
	{
		CI_LOG_W("No Open Fixture Library found at path '" << path.string() << "'");
		return false;
	}
	m_ofl.libraryPath = path;
	CI_LOG_I("Found Open Fixture Library:" << m_ofl.libraryPath);
	return true;
}

bool act::room::OFLDescriptionMapper::getIsParsed()
{
	return m_ofl.isParsed;
}

std::vector<act::room::OFLDescriptionMapper::OFLManufacturerRef> act::room::OFLDescriptionMapper::getManufacturers(bool allowParsingMeta)
{
	// If the libaray is not parsed jet but we are allowed to we parse it first
	if (!m_ofl.isParsed && allowParsingMeta)
		parseLibraryMeta();

	return m_ofl.manufacturers;

}

bool act::room::OFLDescriptionMapper::parseLibraryMeta()
{
	ci::Json manufacturersJson;
	fs::path manufacturersDescriptionPath = m_ofl.libraryPath;
	manufacturersDescriptionPath.append(m_ManufacturerIdxFileName);
	try
	{
		manufacturersJson = ci::loadJson(loadFile(manufacturersDescriptionPath));
	}
	catch (cinder::Exception e) {
		CI_LOG_E(e.what());
		return false;
	}

	m_ofl.isParsed = false;
	m_ofl.manufacturers.clear();

	for (Json::iterator it = manufacturersJson.begin(); it != manufacturersJson.end(); ++it)
	{
		if (!it.value().is_object())
			continue;

		if (!it.value()["name"].is_string()) {
			CI_LOG_E("OFLImport: Error while parsing ofl manufacturers.json! Manufacturer name at position " << it.key() << " is not a string.");
			continue;
		}

		OFLManufacturer oflManufacturer = {
			.key = it.key(),
			.name = it.value()["name"]
		};

		// check if the key corresponds to a directors (as it should)
		fs::path manufacturerDirectoryPath = m_ofl.libraryPath;
		manufacturerDirectoryPath.append(oflManufacturer.key);
		if (!fs::is_directory(manufacturerDirectoryPath))
		{
			CI_LOG_E("OFLImport: Could not find directory: "<< manufacturerDirectoryPath);
			continue;
		}

		// iterate files in the directors and use their names as fixture names
		// this might not be the real fixture name but otherwise all json descriptons
		// would have to be parsed, most of which will not be needed
		for (const auto& entry : fs::directory_iterator(manufacturerDirectoryPath))
		{
			if (!entry.path().has_extension() || entry.path().extension() != ".json")
				continue;

			// aproximating fixture name out of filename
			std::string filename = entry.path().filename().stem().string();
			std::replace(filename.begin(), filename.end(), '_', ' ');
			std::replace(filename.begin(), filename.end(), '-', ' ');

			OFLFixtureDescription oflDescription = {
				.uid = oflManufacturer.key + "-" + filename,
				.name = filename,
				.descriptionPath = entry.path()
			};

			oflManufacturer.fixtures.push_back(std::make_shared<OFLFixtureDescription>(oflDescription));
		}

		m_ofl.manufacturers.push_back(std::make_shared<OFLManufacturer>(oflManufacturer));
	}

	m_ofl.isParsed = true;

	return true;
}

bool act::room::OFLDescriptionMapper::parseBasicDescription(OFLFixtureDescriptionRef fixtureDescription)
{
	fixtureDescription->externalDescription.clear();
	fixtureDescription->modes.clear();
	fixtureDescription->hasError = true;

	if (!fixtureDescription->descriptionPath.has_extension() || fixtureDescription->descriptionPath.extension() != ".json")
		throw std::invalid_argument("OFLDescriptionMapper::parseBasicDescription: Could not load fixture description because path does not lead to a json file!");

	try
	{
		fixtureDescription->externalDescription = ci::loadJson(loadFile(fixtureDescription->descriptionPath));
	}
	catch (const std::exception& e)
	{
		CI_LOG_E("Could not load fixture description for path " << fixtureDescription->descriptionPath << ": " << e.what());
		return false;
	}

	ci::Json& externalDesc = fixtureDescription->externalDescription;

	if (!externalDesc.is_object()) 
		return false;

	if (!externalDesc.contains("availableChannels") || !externalDesc["availableChannels"].is_object())
		return false;

	// Check and set category
	if (!externalDesc.contains("categories") || !externalDesc["categories"].is_array())
		return false;

	// Check and set modes and corresponding channels
	if (!externalDesc.contains("modes") || !externalDesc["modes"].is_array())
		return false;

	for (const auto& modeDesc : externalDesc["modes"])
	{
		if (!modeDesc.is_object() || !modeDesc.contains("name") || !modeDesc.contains("channels") || !modeDesc["channels"].is_array())
			return false;

		OFLMode oflMode = {
			.name = modeDesc["name"]
		};
		fixtureDescription->modes.push_back(std::make_shared<OFLMode>(oflMode));
	}

	try
	{
		translateOFLtoInternal(fixtureDescription);
		fixtureDescription->hasError = false;
	}
	catch (const std::exception& e)
	{
		fixtureDescription->hasError = true;
		CI_LOG_E("Could not translate fixture: '" << fixtureDescription->name << "'! : " << e.what());
		return false;
	}

	return true;
}

// Takes a OFLFixtureDescription and tries to translate the parameters of the available modes into the inACTually internal fixture format
bool act::room::OFLDescriptionMapper::translateOFLtoInternal(OFLFixtureDescriptionRef fixtureDescription)
{
	auto const& externalDesc = fixtureDescription->externalDescription;

	// Type
	if (!externalDesc.contains("categories") || !externalDesc["categories"].is_array())
		throw std::invalid_argument("external description does not contain array of fixture categories!");

	// Check if it is a laser and abort transltion if it is
	if (isInJsonArray("Laser", externalDesc["categories"]))
	{
		fixtureDescription->type = "laser";
		fixtureDescription->isSupportedType = false;
		throw std::exception("Fixture is or contains a laser! InACTually currently can not handle lasers so no translation will be performed.");
	}

	// Check if it is one of the supported types
	if (externalDesc["categories"].size() == 1 && externalDesc["categories"][0] == "Dimmer")
		fixtureDescription->type = "dimmer";
	else if (isInJsonArray("Moving Head", externalDesc["categories"]) || isInJsonArray("Color Changer", externalDesc["categories"]))
		fixtureDescription->type = "mv";
	else if (fixtureDescription->forceTranslation)
		fixtureDescription->type = "mv";
	else
		throw std::exception("Fixture type is not supported");

	fixtureDescription->isSupportedType = true;

	for (int modeIndex = 0; modeIndex < fixtureDescription->modes.size(); modeIndex++)
	{
		OFLModeRef modeRef = fixtureDescription->modes.at(modeIndex);
		modeRef->internalDescBase = ci::Json::object();

		//Fixture Name (always mode specific)
		if (!externalDesc.contains("name") || !externalDesc["name"].is_string()) 
			{ CI_LOG_E("OFL fixture import failed! No Name in external Description found."); return false; }
		std::string fixtureName = externalDesc["name"];
		modeRef->internalDescBase["name"] = fixtureName + " (" + modeRef->name + " mode)";

		modeRef->internalDescBase["type"] = fixtureDescription->type;

		// Determine number of channels
		if (!externalDesc.contains("modes") || externalDesc["modes"].size() - 1 < modeIndex)
			throw std::invalid_argument("Mode not found in external Description.");
		if (!externalDesc["modes"][modeIndex].contains("channels") || !externalDesc["modes"][modeIndex]["channels"].is_array())
			throw std::invalid_argument("No ChannelArray found for selected mode.");
		modeRef->internalDescBase["channel"] = externalDesc["modes"][modeIndex]["channels"].size();
		
		// Translate channels for selected mode to mapping
		// adding other information to the description if needed
		if (!externalDesc.contains("availableChannels") || !externalDesc["availableChannels"].is_object())
			throw std::invalid_argument("AvailableChannels missing in externalDescription");

		for (int dmxOffset = 0; dmxOffset < externalDesc["modes"][modeIndex]["channels"].size(); dmxOffset++)
		{
			if (!externalDesc["modes"][modeIndex]["channels"][dmxOffset].is_string())
			{
				CI_LOG_W("Unexpected channels format in external fixture defifition! Skipping non-string entries.");
				continue;
			}

			const std::string& channelKey = externalDesc["modes"][modeIndex]["channels"][dmxOffset];

			OFLChannelDescPatchRef descPatchRef = std::make_shared<OFLChannelDescPatch>();
			descPatchRef->includePatch = false;

			// Check if there is already a translation for this channel
			// checkOtherAffectedChannels could produce that
			OFLChannelTranslationRef channelTranslationRef;
			if (!modeRef->channelTranslationMapping.contains(dmxOffset))
			{
				OFLChannelTranslation channelTranslation;
				channelTranslation.oflChannelKey = channelKey;
				channelTranslation.channelDescPatch = descPatchRef;

				channelTranslationRef = std::make_shared<OFLChannelTranslation>(channelTranslation);
				modeRef->channelTranslationMapping.insert({ dmxOffset, channelTranslationRef });
			}
			else
			{
				channelTranslationRef = modeRef->channelTranslationMapping.at(dmxOffset);
				if (channelTranslationRef->oflChannelKey == "unknown")
					channelTranslationRef->oflChannelKey = channelKey;
			}

			// Channels are arbitrary keys so we need to lookup type of the capability which should follow a standardised naming
			// So lets check if the channel key is in the available channels and has a capability with a type
			if (!externalDesc["availableChannels"].contains(channelKey))
			{
				CI_LOG_W("Could not find channl key '" << channelKey << "' in availableChannels! Skipping channel.");
				continue;
			}

			ci::Json internalDescPatch = ci::Json::object();

			try
			{
				internalDescPatch = translateChannel(externalDesc["availableChannels"][channelKey], externalDesc, dmxOffset + 1, modeIndex, channelKey);
			}
			catch (const std::exception& e)
			{
				CI_LOG_W("Could not translate channel '" << channelKey << "' :" << e.what());
			}
			

			if (internalDescPatch.is_null())
			{
				CI_LOG_E("Json Patch is Null! This would destroy the internalDescription and is therefore ignored! channelkey:" << channelKey);
				continue;
			}

			if (fixtureDescription->forceTranslation)
				modeRef->internalDescBase["notes"]["forcedTranslation"] = "CHECK TRANSLATION BEFORE USING THE DESCRIPTION! Fixture type is not a supported fixture type. Translation was forced by the user.";

			descPatchRef->descriptionPatch = internalDescPatch;

			addDescToOtherAffectedChannels(modeRef, descPatchRef, channelKey, dmxOffset);

			if (!internalDescPatch.empty() && !internalDescPatch.contains("notes"))
				descPatchRef->includePatch = true;
			else
				CI_LOG_W("Channel " << channelKey << " wont be included in final translation by default because it's translation resulted in notes.");
		}
	}
	return true;
}

ci::Json act::room::OFLDescriptionMapper::getInternalDescription(OFLFixtureDescriptionRef fixture)
{
	OFLModeRef modeRef = fixture->modes.at(fixture->selectedMode);
	if (modeRef->internalDescBase.empty() || !modeRef->internalDescBase.contains("name"))
	{
		CI_LOG_E("Malformed internalDescBase provided, no internal description producable!");
		return ci::Json::object();
	}

	ci::Json internalDescription = modeRef->internalDescBase;

	for (auto const& [channel, channelTranslation] : modeRef->channelTranslationMapping)
	{
		//TODO check constraints like only one dimmer etc.
		if (channelTranslation->channelDescPatch->includePatch)
			internalDescription.merge_patch(channelTranslation->channelDescPatch->descriptionPatch);
	}

	return internalDescription;
}

void act::room::OFLDescriptionMapper::addDescToOtherAffectedChannels(OFLModeRef modeRef, OFLChannelDescPatchRef channelDescPatchRef, std::string const& channelKey, int primaryDmxOffset)
{
	// Checks all channels in mapping of description patch and adds the description ref to the translation 
	// if no translation exists jet
	// the mapping with the primaryDMXOffset will be ignored because it should already have the description

	// Loop over Mapping and add TranslationRef to corresponding TranslationMappings
	if (channelDescPatchRef->descriptionPatch.is_null() || channelDescPatchRef->descriptionPatch.empty())
		return;

	if (!channelDescPatchRef->descriptionPatch.contains("mapping") || !channelDescPatchRef->descriptionPatch["mapping"].is_object()) {
		CI_LOG_E("Internal Description does not contain mapping object! channelKey:" << channelKey);
		return;
	}

	for (auto const& [parameter, inACTparamOffset] : channelDescPatchRef->descriptionPatch["mapping"].items())
	{
		if (!inACTparamOffset.is_number())
		{
			CI_LOG_E("Can not add internalDesc to all mappings because mapping value is not a number for parameter " << parameter << " of " << channelKey);
			return;
		}

		int paramDmxOffset = inACTparamOffset - 1; // arrays etc start with 0, inACTually uses 1 as first address

		if (paramDmxOffset == primaryDmxOffset)
			continue; // The translation is set already by the translateOFLtoInternal function

		if (modeRef->channelTranslationMapping.contains(paramDmxOffset))
		{
			// There is already a channelTranslation registered at this offset
			OFLChannelTranslationRef channelTranslationRef = modeRef->channelTranslationMapping.at(paramDmxOffset);

			// Check if it has already an internalDescription
			if (channelTranslationRef->channelDescPatch && !channelTranslationRef->channelDescPatch->descriptionPatch.empty())
			{
				// we already have a desription and it is not empty. This should not happen
				// So we dont include both by default and add a warning
				std::string note = "The parameter has two conflicting translations! Caused by parameter '" + parameter + "'!";
				channelDescPatchRef->includePatch = false;
				attachNoteToChannelDesc(channelDescPatchRef->descriptionPatch, inACTparamOffset, note);

				channelTranslationRef->channelDescPatch->includePatch = false;
				attachNoteToChannelDesc(channelTranslationRef->channelDescPatch->descriptionPatch, inACTparamOffset, note);
				continue;
			}

			// if there is no channel Descirption we can add the new one
			channelTranslationRef->channelDescPatch = channelDescPatchRef;
		}
		else
		{
			// otherwise we need to create a new channel translation and append that
			OFLChannelTranslationRef translationRef = std::make_shared<OFLChannelTranslation>();
			translationRef->oflChannelKey = "unknown";
			translationRef->channelDescPatch = channelDescPatchRef;
			modeRef->channelTranslationMapping.insert({ paramDmxOffset, translationRef });
		}
	}
}

void act::room::OFLDescriptionMapper::attachNoteToChannelDesc(ci::Json& internalDescPatch, int dmxOffset, std::string note)
{
	if (!internalDescPatch.contains("notes"))
		internalDescPatch["notes"] = ci::Json::object();

	std::string channelKey = "channel-" + std::to_string(dmxOffset);

	if (internalDescPatch["notes"].contains(channelKey) && !internalDescPatch["notes"][channelKey].is_array())
	{
		// There is already a notes list for that channel but it is not an array
		CI_LOG_E("could not attach note to channel " << channelKey << " because there are notes but not as array! note:"<<note);
		return;
	}
	else
	{
		// There is no notes list jet so we create an array
		internalDescPatch["notes"][channelKey] = ci::Json::array();
	}
	internalDescPatch["notes"][channelKey].push_back(note);
}

std::string act::room::OFLDescriptionMapper::getWheelNameKey(std::string const& channelName, ci::Json const& extCapabilities)
{
	// WheelName can be channel name or stated explicitly in the description
	return (extCapabilities.contains("wheel") && extCapabilities["wheel"].is_string()) ? extCapabilities["wheel"].get<std::string>() : channelName;
}

int act::room::OFLDescriptionMapper::convertDegStrToInt(std::string degStr, std::string decimalpoint)
{
	// NOTE: angles in OFL can also be in % but since inACTually currently can not handle 
	// proportional angles it wont be converted but result in an exception

	size_t degPos = degStr.find("deg");
	
	if (degPos == std::string::npos) throw std::invalid_argument("degStr does not contain 'deg'!");
	if (degStr.length() - degPos != 3) throw std::invalid_argument("degStr does not contain 'deg' as last three characters!");

	degStr.erase(degStr.length() - 3, 3);

	if (degStr.find(decimalpoint) != std::string::npos)
		return static_cast<int>(std::round(std::stof(degStr)));
	else
		return std::stoi(degStr);
}

float act::room::OFLDescriptionMapper::speedToFloat(std::string speed)
{
	// Speed can have Hz, bpm or % as units
	size_t HzPos = speed.find("Hz");
	if (HzPos != std::string::npos)
		speed.erase(HzPos, 2);

	size_t bpmPos = speed.find("bpm");
	if (bpmPos != std::string::npos)
		speed.erase(bpmPos, 3);

	size_t percPos = speed.find("%");
	if (percPos != std::string::npos)
		speed.erase(percPos, 1);

	return std::stof(speed);
}

int act::room::OFLDescriptionMapper::isInJsonObjArray(std::string key, std::string value, ci::Json const& array)
{
	if (!array.is_array()) return -1;

	bool matchKey = !key.empty();
	bool matchValue = !value.empty();

	for (int idx = 0; idx < array.size(); idx++)
	{
		for (auto const& [elemKey, elemValue] : array[idx].items())
		{
			if (!elemValue.is_string())
				continue;
			if ((matchKey && key == elemKey) && (matchValue && value == elemValue)) return idx; // return if key and value match
			else if (!matchKey && matchValue && value == elemValue) return idx; // return if value matches and no key was provided
			else if (!matchValue && matchKey && key == elemKey) return idx; // return if key matches and no value was provided
		}
	}

	return -1;
}

bool act::room::OFLDescriptionMapper::isInJsonArray(std::string value, ci::Json const& array)
{
	if (!array.is_array())
		return false;
	for (auto const& entry : array)
	{
		if (!entry.is_string())
			continue;
		if (entry == value)
			return true;
	}
	return false;
}

int act::room::OFLDescriptionMapper::dmxRangeDistance(ci::Json const& distantRange, ci::Json const& baseRange)
{
	if (!distantRange.is_array() || !baseRange.is_array()
		|| distantRange.size() != 2 || baseRange.size() != 2)
		throw new std::invalid_argument("Malformed dmxRanges!");

	if (distantRange[1] < baseRange[0])
		return distantRange[1].get<int>() - baseRange[0].get<int>(); // distantRange below baseRange
	if (baseRange[1] < distantRange[0])
		return distantRange[0].get<int>() - baseRange[1].get<int>(); // disntantRange above baseRange 

	return 0; // distantRange within baseRange (/or overlapping)
}

int act::room::OFLDescriptionMapper::dmxRangeToDmxValue(ci::Json const& dmxRange)
{
	if (!dmxRange.is_array() || dmxRange.size() != 2)
		throw std::invalid_argument("Malformed dmxRange!");

	return std::round((dmxRange[0].get<int>() + dmxRange[1].get<int>()) * 0.5);
}

std::map<std::string, int> act::room::OFLDescriptionMapper::resolveFineChannels(ci::Json const& fullExtDesc, int modeIndex, ci::Json const& extChannelDesc, int maxAliases)
{
	if (!fullExtDesc.contains("modes") || !fullExtDesc["modes"].is_array() || fullExtDesc["modes"].size() <= modeIndex
		|| !fullExtDesc["modes"][modeIndex].contains("channels") || !fullExtDesc["modes"][modeIndex]["channels"].is_array())
	{
		CI_LOG_E("resolveFineChannel got malformed fullExtDesc! Skipping fine channel resolution.");
		return std::map<std::string, int>();
	}

	if (!extChannelDesc.contains("fineChannelAliases") || !extChannelDesc["fineChannelAliases"].is_array())
		return std::map<std::string, int>(); // Nothing to resolve

	std::map<std::string, int> retLookup;

	for (int i = 0; i < extChannelDesc["fineChannelAliases"].size(); i++)
	{
		if (i > maxAliases - 1)
		{
			CI_LOG_W("Found to many fineChannelAliases! Skipping all above " << i);
			break;
		}

		if (extChannelDesc["fineChannelAliases"][i].is_string())
		{
			std::string fineChannelAlias = extChannelDesc["fineChannelAliases"][i];
			auto const& channelList = fullExtDesc["modes"][modeIndex]["channels"];
			
			auto iter = std::find(channelList.begin(), channelList.end(), fineChannelAlias);
			if (iter != channelList.end())
			{
				retLookup.insert({ fineChannelAlias, std::distance(channelList.begin(), iter)});
			}
			else
			{
				retLookup.insert({ fineChannelAlias, -1 });
				CI_LOG_W("Could not resolve fineChannelAlias: " << fineChannelAlias);
			}
		}
		else
			CI_LOG_W("Found non string fineChannelAlias! Skipping fine channel translation.");
	}

	return retLookup;
}

// Determine the capability or capabilities of a channel and call the correct translate function
// Register new translation in this method.
// For a list of current capability types see https://github.com/OpenLightingProject/open-fixture-library/blob/master/docs/capability-types.md
ci::Json act::room::OFLDescriptionMapper::translateChannel(ci::Json const& extChannelDesc, ci::Json const& fullExtDesc, int dmxOffset, int modeIndex, std::string const& channelName)
{
	if (extChannelDesc.contains("capability")) // === Translate single capability
	{
		ci::Json const& extCapabilityDesc = extChannelDesc["capability"];

		if (!extCapabilityDesc.contains("type") || !extCapabilityDesc["type"].is_string())
			throw std::invalid_argument("Capability has to have a type attribute that is a string!");

		std::string const& capabilityType = extCapabilityDesc["type"];

		if (capabilityType == "NoFunction")
			return ci::Json::object(); // NoFunction can be ignored except for some special cases which should not matter for our purpose
		else if (capabilityType == "Intensity")
			return translateIntensityCapability(extCapabilityDesc, extChannelDesc, fullExtDesc, dmxOffset, modeIndex);
		else if (capabilityType == "Pan" || capabilityType == "Tilt")
			return translatePanTiltCapability(extCapabilityDesc, extChannelDesc, fullExtDesc, dmxOffset, modeIndex);
		else if (capabilityType == "Zoom")
			return translateZoomCapability(extCapabilityDesc, extChannelDesc, fullExtDesc, dmxOffset, modeIndex);
		else if (capabilityType == "ColorIntensity")
			return translateColorIntensityCapability(extCapabilityDesc, extChannelDesc, fullExtDesc, dmxOffset, modeIndex);
		else if (capabilityType == "PanTiltSpeed")
			return translatePanTiltSpeedCapability(extCapabilityDesc, extChannelDesc, fullExtDesc, dmxOffset, modeIndex);
	}
	else if (extChannelDesc.contains("capabilities")) // === Translate capabilities list
	{
		ci::Json const& extCapabilitiesDesc = extChannelDesc["capabilities"];

		if (isColorWheel(channelName, extChannelDesc, fullExtDesc, dmxOffset, modeIndex))
			return translateColorWheelCapability(extCapabilitiesDesc, extChannelDesc, fullExtDesc, dmxOffset, modeIndex, channelName);
		else if (isGoboWheel(channelName, extChannelDesc, fullExtDesc, dmxOffset, modeIndex))
			return translateGoboWheelCapability(extCapabilitiesDesc, extChannelDesc, fullExtDesc, dmxOffset, modeIndex, channelName);
		else if (isShutterStrobeCapability(channelName, extChannelDesc, fullExtDesc, dmxOffset, modeIndex))
			return translateShutterStrobeCapability(extCapabilitiesDesc, extChannelDesc, fullExtDesc, dmxOffset, modeIndex, channelName);
	}

	CI_LOG_W("No mapping exists for channel '" << channelName << "'");

	return ci::Json::object();
}

ci::Json act::room::OFLDescriptionMapper::translateIntensityCapability(ci::Json const& extCapabilityDesc, ci::Json const& extChannelDesc, ci::Json const& fullExtDesc, int dmxOffset, int modeIndex)
{
	ci::Json descPatch = ci::Json::object();

	descPatch["mapping"]["dimmer"] = dmxOffset;

	auto const& fineChannelMap = resolveFineChannels(fullExtDesc, modeIndex, extChannelDesc, 1);
	if (fineChannelMap.size() == 1 && fineChannelMap.begin()->second >= 0)
		descPatch["mapping"]["fineDimmer"] = fineChannelMap.begin()->second + 1; //Add +1 because inACTually starts at 1 but channels index starts at 0

	return descPatch;
}

ci::Json act::room::OFLDescriptionMapper::translatePanTiltCapability(ci::Json const& extCapabilityDesc, ci::Json const& extChannelDesc, ci::Json const& fullExtDesc, int dmxOffset, int modeIndex)
{
	// The translation of Pan and Tilt is so similar it is combined into one function

	ci::Json descPatch = ci::Json::object();

	if (!extCapabilityDesc.contains("type") || (extCapabilityDesc["type"] != "Pan" && extCapabilityDesc["type"] != "Tilt"))
		throw std::exception("translatePanTiltCapability called but capability is neither pan or tilt!");

	std::string panTiltIdentifier = extCapabilityDesc["type"]; // panTileIdentifier holds the information if we are translating pan or tilt
	std::string panTiltIdentifierL = panTiltIdentifier; // panTiltIdentifierL is the lowercase equivalent
	std::transform(panTiltIdentifierL.begin(), panTiltIdentifierL.end(), panTiltIdentifierL.begin(), [](unsigned char c) {return std::tolower(c);});

	// Check angle start and angle end to calculate Range
	if (!extCapabilityDesc.contains("angleStart") || !extCapabilityDesc["angleStart"].is_string()) 
		throw std::exception("No angleStart in extCapabilityDesc!");
	int angleStart = convertDegStrToInt(extCapabilityDesc["angleStart"]);

	if (!extCapabilityDesc.contains("angleEnd") || !extCapabilityDesc["angleEnd"].is_string())
		throw std::exception("No angleEnd in extCapabilityDesc!");
	int angleEnd = convertDegStrToInt(extCapabilityDesc["angleEnd"]);

	if (angleEnd < angleStart) throw std::exception("angleStart is larger that angleEnd in pan or tilt capability!");

	descPatch[panTiltIdentifierL +"Range"] = angleEnd - angleStart;

	descPatch["mapping"][panTiltIdentifierL] = dmxOffset;

	auto const& fineChannelMap = resolveFineChannels(fullExtDesc, modeIndex, extChannelDesc, 1);
	if (fineChannelMap.size() == 1 && fineChannelMap.begin()->second >= 0)
		descPatch["mapping"]["fine"+ panTiltIdentifier] = fineChannelMap.begin()->second + 1; //Add +1 because inACTually starts at 1 but channels index starts at 0

	return descPatch;
}

ci::Json act::room::OFLDescriptionMapper::translateZoomCapability(ci::Json const& extCapabilityDesc, ci::Json const& extChannelDesc, ci::Json const& fullExtDesc, int dmxOffset, int modeIndex)
{
	ci::Json descPatch = ci::Json::object();

	if (!extCapabilityDesc.contains("angleStart") || !extCapabilityDesc["angleStart"].is_string())
		throw std::exception("No angleStart in extCapabilityDesc!");
	int angleStart = convertDegStrToInt(extCapabilityDesc["angleStart"]);

	if (!extCapabilityDesc.contains("angleEnd") || !extCapabilityDesc["angleEnd"].is_string())
		throw std::exception("No angleEnd in extCapabilityDesc!");
	int angleEnd = convertDegStrToInt(extCapabilityDesc["angleEnd"]);

	if (angleStart > angleEnd)
	{
		descPatch["beamAngleMax"] = angleStart;
		descPatch["beamAngleMin"] = angleEnd;
	}
	else 
	{
		descPatch["beamAngleMax"] = angleEnd;
		descPatch["beamAngleMin"] = angleStart;
	}

	descPatch["beamAngle"] = descPatch["beamAngleMin"];

	descPatch["mapping"]["zoom"] = dmxOffset;

	auto const& fineChannelMap = resolveFineChannels(fullExtDesc, modeIndex, extChannelDesc, 1);
	if (fineChannelMap.size() == 1 && fineChannelMap.begin()->second >= 0)
		descPatch["mapping"]["fineZoom"] = fineChannelMap.begin()->second + 1; //Add +1 because inACTually starts at 1 but channels index starts at 0

	return descPatch;
}

ci::Json act::room::OFLDescriptionMapper::translateColorIntensityCapability(ci::Json const& extCapabilityDesc, ci::Json const& extChannelDesc, ci::Json const& fullExtDesc, int dmxOffset, int modeIndex)
{
	ci::Json descPatch = ci::Json::object();

	if (!extCapabilityDesc.contains("color") || !extCapabilityDesc["color"].is_string())
		throw std::invalid_argument("Capability Description does not contain color entry or color entry is not a string!");

	// Translation from OFL Key to internal key
	std::map<std::string, std::string> translationLookup = { {"Red", "R"}
															,{"Green", "G"}
															,{"Blue", "B"}
															,{"Amber", "A"}
															,{"White", "W"}
															,{"UV", "UV"}
															};

	if (translationLookup.find(extCapabilityDesc["color"]) != translationLookup.end())
	{
		std::string color = translationLookup.at(extCapabilityDesc["color"]);
		descPatch["mapping"][color] = dmxOffset;

		auto const& fineChannelMap = resolveFineChannels(fullExtDesc, modeIndex, extChannelDesc, 1);
		if (fineChannelMap.size() == 1 && fineChannelMap.begin()->second >= 0)
			descPatch["mapping"]["fine" + color] = fineChannelMap.begin()->second + 1; //Add +1 because inACTually starts at 1 but channels index starts at 0
	}
	else
		CI_LOG_W("Color type '" << extCapabilityDesc["color"] << "' is not supported.");


	// Not resolving brightnessStart and brightnessEnd because InACTually currently can not use them

	return descPatch;
}

ci::Json act::room::OFLDescriptionMapper::translatePanTiltSpeedCapability(ci::Json const& extCapabilityDesc, ci::Json const& extChannelDesc, ci::Json const& fullExtDesc, int dmxOffset, int modeIndex)
{
	ci::Json descPatch = ci::Json::object();
	
	descPatch["mapping"]["speed"] = dmxOffset;

	return descPatch;
}

// Checks if there is a capability with array of type wheelSlot and a corresponding wheel description with slots of type color
bool act::room::OFLDescriptionMapper::isColorWheel(std::string const& channelName, ci::Json const& extChannelDesc, ci::Json const& fullExtDesc, int dmxOffset, int modeIndex)
{
	if (!extChannelDesc.contains("capabilities") && !extChannelDesc["capabilities"].is_array())
		return false; // A Color Wheeel has to have an array of WheelSlot capabilities

	int idx = isInJsonObjArray("type", "WheelSlot", extChannelDesc["capabilities"]);

	if (idx < 0)
		return false; // No WheelSlot in capabilities

	std::string WheelName = getWheelNameKey(channelName, extChannelDesc["capabilities"][idx]);

	if (!fullExtDesc.contains("wheels") || !fullExtDesc["wheels"].contains(WheelName) || !fullExtDesc["wheels"][WheelName].contains("slots") || !fullExtDesc["wheels"][WheelName]["slots"].is_array())
		return false; // Color wheels always have a seperate description holding the information about the colors

	if(isInJsonObjArray("type", "Color", fullExtDesc["wheels"][WheelName]["slots"]) < 0)
		return false; // if no slots of the wheel holds color it is not a color wheel
	
	return true; // Otherwise it is a color wheel
}

ci::Json act::room::OFLDescriptionMapper::translateColorWheelCapability(ci::Json const& extCapabilityDesc, ci::Json const& extChannelDesc, ci::Json const& fullExtDesc, int dmxOffset, int modeIndex, std::string const& channelName)
{

	if (!extChannelDesc.contains("capabilities") ||  !extChannelDesc["capabilities"].is_array())
		throw std::invalid_argument("A Color Wheeel has to have an array of WheelSlot capabilities");

	if (!fullExtDesc.contains("wheels") || !fullExtDesc["wheels"].is_object())
		throw std::invalid_argument("For a Color Wheel a wheels object has to be present in the external description");

	std::list<std::string> notes = std::list<std::string>();

	ci::Json descPatch = ci::Json::object();

	ci::Json colorMap = ci::Json::object();
	colorMap["colors"] = ci::Json::array();

	int amount = 0;
	int start = -1;
	int end = -1;

	for (auto const& [idx, capability] : extChannelDesc["capabilities"].items())
	{
		if (!capability.contains("type") || !capability["type"].is_string())
		{
			CI_LOG_W("Found no or non string type in capability of color wheel translation! Skipping capability.");
			continue;
		}
		if(capability["type"] != "WheelSlot")
		{
			CI_LOG_W("Found non Wheel Slot Type '" << capability["type"] << "' in color wheel translation! Skipping capability");
			continue;
		}

		std::string WheelName = (capability.contains("wheel") && capability["wheel"].is_string()) ? capability["wheel"].get<std::string>() : channelName;

		if (!fullExtDesc["wheels"].contains(WheelName))
			throw std::invalid_argument("Wheel with name '" + WheelName + "' could not be found in wheels object.");

		if (!capability.contains("dmxRange") || !capability["dmxRange"].is_array() || capability["dmxRange"].size() != 2
			|| !capability["dmxRange"][0].is_number() || !capability["dmxRange"][1].is_number())
			throw std::invalid_argument("Malformed dmxRange in capability!");

		int dmxValue = dmxRangeToDmxValue(capability["dmxRange"]);

		if (!capability.contains("slotNumber") || !capability["slotNumber"].is_number())
		{
			CI_LOG_W("Color Wheel translation WheelSlot at position " << idx << " does not specify single slotNumbers! Half frames currently not supported, skipping slot.");

			int dmxMin = capability["dmxRange"][0];
			int dmxMax = capability["dmxRange"][1];

			if (capability.contains("slotNumberStart") && capability.contains("slotNumberEnd"))
				notes.push_back("Dmx range " + std::to_string(dmxMin) + " - " + std::to_string(dmxMax) + " unavailable, split slots not supported.");
			else
				notes.push_back("Dmx range " + std::to_string(dmxMin) + " - " + std::to_string(dmxMax) + " unavailable, no unique slot identified.");

			continue;
		}

		int slotIdx = capability["slotNumber"] - 1; //SlotNumber starts at 1 but is referencing an array

		if (!fullExtDesc["wheels"][WheelName].is_object() || !fullExtDesc["wheels"][WheelName].contains("slots") || !fullExtDesc["wheels"][WheelName]["slots"].is_array())
			throw std::invalid_argument("Malformed wheels wheel '" + WheelName + "'");

		if (slotIdx >= fullExtDesc["wheels"][WheelName]["slots"].size())
			throw std::invalid_argument("Channel '" + channelName + "' specifies slotNumber greater than available slots in wheels description!");

		auto const& wheelSlotDesc = fullExtDesc["wheels"][WheelName]["slots"][slotIdx];

		if (!wheelSlotDesc.contains("type") || !wheelSlotDesc["type"].is_string() || wheelSlotDesc["type"] != "Color")
		{
			CI_LOG_W("Channel '" << channelName << "' with wheel '" << WheelName << "' at slotNumber '" << slotIdx << "' is not of type 'Color'! Skipping slot.");
			continue;
		}

		if (!wheelSlotDesc.contains("name") || !wheelSlotDesc["name"].is_string())
		{
			CI_LOG_W("Channel '" << channelName << "' with wheel '" << WheelName << "' at slotNumber '" << slotIdx << "' does not contain a name! Skipping slot.");
			continue;
		}

		if (!wheelSlotDesc.contains("colors") || !wheelSlotDesc["colors"].is_array() || wheelSlotDesc["colors"].size() < 1)
		{
			CI_LOG_W("Channel '" << channelName << "' with wheel '" << WheelName << "' at slotNumber '" << slotIdx << "' does not contain a or an empty colors array! Skipping slot.");
			continue;
		}

		auto const& colorHex = wheelSlotDesc["colors"][0].get<std::string>();
		ci::Color const& colorRGB = RGBAWHelper::HEXtoRGB(colorHex);

		ci::Json colorObj = ci::Json::object();
		colorObj["value"] = dmxValue;
		colorObj["label"] = wheelSlotDesc["name"];
		colorObj["hex"] = colorHex;
		colorObj["R"] = (uint8_t)(colorRGB.r * 255);
		colorObj["G"] = (uint8_t)(colorRGB.g * 255);
		colorObj["B"] = (uint8_t)(colorRGB.b * 255);

		colorMap["colors"].push_back(colorObj);
		amount++;

		if (start == -1 || dmxValue < start)
			start = dmxValue;
		if (dmxValue > end)
			end = dmxValue;
	}

	if (amount < 1 || start < 0 || end < 0 || start > end)
		throw std::exception("Amount, start and/or end are not plausible after translation!");

	colorMap["amount"] = amount;
	colorMap["start"] = start;
	colorMap["end"] = end;

	descPatch["colorMap"] = colorMap;
	descPatch["mapping"]["color"] = dmxOffset;

	if (!notes.empty())
		descPatch["notes"]["channel-" + std::to_string(dmxOffset)] = notes;

	return descPatch;
}

bool act::room::OFLDescriptionMapper::isGoboWheel(std::string const& channelName, ci::Json const& extChannelDesc, ci::Json const& fullExtDesc, int dmxOffset, int modeIndex)
{
	if (!extChannelDesc.contains("capabilities") && !extChannelDesc["capabilities"].is_array())
		return false; // A Gobo Wheeel has to have an array of WheelSlot capabilities

	int idx = isInJsonObjArray("type", "WheelSlot", extChannelDesc["capabilities"]);

	if (idx < 0)
		return false; // No WheelSlot in capabilities

	// WheelName can be channel name or stated explicitly in the description
	std::string WheelName = getWheelNameKey(channelName, extChannelDesc["capabilities"][idx]);

	if (!fullExtDesc.contains("wheels") || !fullExtDesc["wheels"].contains(WheelName) || !fullExtDesc["wheels"][WheelName].contains("slots") || !fullExtDesc["wheels"][WheelName]["slots"].is_array())
		return false; // Gobo wheels always have a seperate description holding the information about the gobos

	if (isInJsonObjArray("type", "Gobo", fullExtDesc["wheels"][WheelName]["slots"]) < 0)
		return false; // if no slots of the wheel holds gobo it is not a gobo wheel

	return true; // Otherwise it is a gobo wheel
}

ci::Json act::room::OFLDescriptionMapper::translateGoboWheelCapability(ci::Json const& extCapabilityDesc, ci::Json const& extChannelDesc, ci::Json const& fullExtDesc, int dmxOffset, int modeIndex, std::string const& channelName)
{

	if (!extChannelDesc.contains("capabilities") || !extChannelDesc["capabilities"].is_array())
		throw std::invalid_argument("A Gobo Wheeel has to have an array of WheelSlot capabilities");

	if (!fullExtDesc.contains("wheels") || !fullExtDesc["wheels"].is_object())
		throw std::invalid_argument("For a Gobo Wheel a wheels object has to be present in the external description");

	std::list<std::string> notes = std::list<std::string>();

	ci::Json descPatch = ci::Json::object();

	int amount = 0;
	int goboStart = -1, goboEnd = -1;
	int shakeStart = -1, shakeEnd = -1;

	for (auto const& [idx, capability] : extChannelDesc["capabilities"].items())
	{
		if (!capability.contains("type") || !capability["type"].is_string())
		{
			CI_LOG_W("Found no or non string type in capability of gobo wheel translation! Skipping capability.");
			continue;
		}
		if (capability["type"] != "WheelSlot" && capability["type"] != "WheelShake")
		{
			CI_LOG_W("Found unexpected type '" << capability["type"] << "' in gobo wheel translation! Skipping capability");
			continue;
		}

		std::string WheelName = (capability.contains("wheel") && capability["wheel"].is_string()) ? capability["wheel"].get<std::string>() : channelName;

		if (!fullExtDesc["wheels"].contains(WheelName))
			throw std::invalid_argument("Wheel with name '" + WheelName + "' could not be found in wheels object.");

		if (!capability.contains("dmxRange") || !capability["dmxRange"].is_array() || capability["dmxRange"].size() != 2
			|| !capability["dmxRange"][0].is_number() || !capability["dmxRange"][1].is_number())
			throw std::invalid_argument("Malformed dmxRange in capability!");

		int dmxMin = capability["dmxRange"][0];
		int dmxMax = capability["dmxRange"][1];
		if (dmxMax < dmxMin)
		{
			dmxMax = dmxMin;
			dmxMin = capability["dmxRange"][1];
		}

		if (!capability.contains("slotNumber") || !capability["slotNumber"].is_number())
		{
			CI_LOG_W("Gobo Wheel capability at position " << idx << " does not specify slotNumber, skipping slot!");
			notes.push_back("Wheel slot " + idx + " unavailable, no slot number provided!");
			continue;
		}

		int slotIdx = capability["slotNumber"] - 1; //SlotNumber starts at 1 but is referencing an array

		if (!fullExtDesc["wheels"][WheelName].is_object() || !fullExtDesc["wheels"][WheelName].contains("slots") || !fullExtDesc["wheels"][WheelName]["slots"].is_array())
			throw std::invalid_argument("Malformed wheels wheel '" + WheelName + "'");

		if (slotIdx >= fullExtDesc["wheels"][WheelName]["slots"].size())
			throw std::invalid_argument("Channel '" + channelName + "' specifies slotNumber greater than available slots in wheels description!");

		auto const& wheelSlotDesc = fullExtDesc["wheels"][WheelName]["slots"][slotIdx];

		if (!wheelSlotDesc.contains("type") || !wheelSlotDesc["type"].is_string() || (wheelSlotDesc["type"] != "Gobo" && wheelSlotDesc["type"] != "Open"))
		{
			CI_LOG_W("Channel '" << channelName << "' with wheel '" << WheelName << "' at slotNumber '" << slotIdx << "' is not of type 'Gobo' or 'Open'! Skipping slot.");
			continue;
		}

		// InACTually currently expects a continious range for gobo selection and wheelshake
		// so we have to ensure the new dmx range is at the lower or upper bound of the knwon range

		if (capability["type"] == "WheelSlot")
		{
			bool foundPlace = false;
			if (goboStart < 0 || (dmxMax + 1) == goboStart)
			{
				// gobo sits below the current range
				goboStart = dmxMin;
				foundPlace = true;
			}
			if (goboEnd < 0 || (dmxMin - 1) == goboEnd)
			{
				// gobo sits above the current range
				goboEnd = dmxMax;
				foundPlace = true;
			}

			if (foundPlace)
				amount++;
			else
			{
				CI_LOG_W("Gobo at slot index " << slotIdx << " does not fit into a continous range of gobos so it will be ignored!");
				notes.push_back("Gobo at slot " + std::to_string(slotIdx + 1) + " (dmx Values " + std::to_string(dmxMin) + " - " + std::to_string(dmxMax) + ") will be unavailable because it's not in a continous dmx gobo range.");
				continue;
			}
		}
		else if (capability["type"] == "WheelShake")
		{
			bool foundPlace = false;
			if (shakeStart < 0 || (dmxMax + 1) == shakeStart)
			{
				// gobo sits below the current range
				shakeStart = dmxMin;
				foundPlace = true;
			}
			if (shakeEnd < 0 || (dmxMin - 1) == shakeEnd)
			{
				// gobo sits above the current range
				shakeEnd = dmxMax;
				foundPlace = true;
			}

			if (!foundPlace)
			{ 
				CI_LOG_W("Wheel shake at slot index " << slotIdx << " does not fit into a continous range of wheek shakes so it will be ignored!");
				notes.push_back("Wheel shake at slot " + std::to_string(slotIdx + 1) + " (dmx Values "+std::to_string(dmxMin) + " - " + std::to_string(dmxMax) + ") will be unavailable because it's not in a continous dmx range.");
				continue;
			}
		}
	}

	if (amount < 1 || goboStart < 0 || goboEnd < 0 || goboEnd < goboStart)
		throw std::exception(("Gobo Wheel translation at dmx offset" + std::to_string(dmxOffset) + " did not result in plausible values, aborting translation!").c_str());

	descPatch["goboMap"] = ci::Json::object();
	descPatch["goboMap"]["amount"] = amount;
	descPatch["goboMap"]["start"] = goboStart;
	descPatch["goboMap"]["end"] = goboEnd;

	if (shakeStart >= 0 && shakeEnd >= 0 && shakeStart < shakeEnd)
	{
		descPatch["goboMap"]["shake-min"] = shakeStart;
		descPatch["goboMap"]["shake-max"] = shakeEnd;
	}

	descPatch["mapping"]["gobo"] = dmxOffset;

	if(!notes.empty())
		descPatch["notes"]["channel-" + std::to_string(dmxOffset)] = notes;

	return descPatch;
}

bool act::room::OFLDescriptionMapper::isShutterStrobeCapability(std::string const& channelName, ci::Json const& extChannelDesc, ci::Json const& fullExtDesc, int dmxOffset, int modeIndex)
{
	// Determine if there are capabilities with an open state and a dedicated strobe state with speedStart and speedEnd
	if (!extChannelDesc.contains("capabilities") && !extChannelDesc["capabilities"].is_array())
		return false;

	bool hasOpenState = false;
	bool hasStrobeWithSpeed = false;

	for (auto const& capability : extChannelDesc["capabilities"])
	{
		if (!capability.contains("type") || capability["type"] != "ShutterStrobe")
			continue;
		
		if (capability.contains("shutterEffect") && capability["shutterEffect"] == "Open")
			hasOpenState = true;
		else if (capability.contains("shutterEffect") && capability["shutterEffect"] == "Strobe"
			&& capability.contains("speedStart") && capability.contains("speedEnd"))
			hasStrobeWithSpeed = true;

		if (hasOpenState && hasStrobeWithSpeed)
			break;
	}

	return hasOpenState && hasStrobeWithSpeed;
}

ci::Json act::room::OFLDescriptionMapper::translateShutterStrobeCapability(ci::Json const& extCapabilityDesc, ci::Json const& extChannelDesc, ci::Json const& fullExtDesc, int dmxOffset, int modeIndex, std::string const& channelName)
{

	if (!extChannelDesc.contains("capabilities") && !extChannelDesc["capabilities"].is_array())
		throw std::invalid_argument("No capabilities array found in capabolities description!");

	ci::Json descPatch = ci::Json::object();

	// NOTE: Currently the strobe parameter of InACTually is rather limited 
	// so the only thing we can translate is a open state as no strobe and an strobe state
	// which is controling the speed directly
	// no strobe type like ramp up etc. or strobe speed controlled with different parameter is
	// possible at the moment

	int strobeIdx = -1;

	// search strobe capability
	for (int idx = 0; idx < extChannelDesc["capabilities"].size(); idx++)
	{
		auto const& capability = extChannelDesc["capabilities"][idx];

		if (!capability.contains("type") || capability["type"] != "ShutterStrobe")
			continue;

		if (capability.contains("shutterEffect") && capability["shutterEffect"] == "Strobe")
		{
			if (capability.contains("speedStart") && capability.contains("speedEnd"))
			{
				strobeIdx = idx;
				break;
			}
		}
	}

	if (strobeIdx < 0)
		throw std::exception(("Encountered incompatible strobe capabilities list at dmxOffset " + std::to_string(dmxOffset)).c_str());

	// find neares open position
	ci::Json const& strobeCapability = extChannelDesc["capabilities"][strobeIdx];
	if (!strobeCapability.contains("dmxRange") || !strobeCapability["dmxRange"].is_array() || strobeCapability["dmxRange"].size() != 2)
		throw std::exception("Could not find well formatted dmxRange in strobe capability!");

	int openStateIdxAndDistance[2] = { -1, -1 };

	for (int idx = 0; idx < extChannelDesc["capabilities"].size(); idx++)
	{
		auto const& capability = extChannelDesc["capabilities"][idx];

		if (!capability.contains("type") || capability["type"] != "ShutterStrobe")
			continue;

		if (capability.contains("shutterEffect") && capability["shutterEffect"] == "Open")
		{
			if (!capability.contains("dmxRange") || !capability["dmxRange"].is_array() || capability["dmxRange"].size() != 2)
			{
				CI_LOG_W("Found capability with ShutterEffect Open but malformed dmxRange! Skipping capability.");
				continue;
			}

			int newDistance = abs(dmxRangeDistance(capability["dmxRange"], strobeCapability["dmxRange"]));
			if (openStateIdxAndDistance[0] < 0 || newDistance < openStateIdxAndDistance[1])
			{
				openStateIdxAndDistance[0] = idx;
				openStateIdxAndDistance[1] = newDistance;
			}


			if (openStateIdxAndDistance[1] == 1)
				break; // If the distance is 1 the OpenState is directly next to the strobeState
					   // So we don't have to look further
		}
	}

	if (openStateIdxAndDistance[0] < 0 || openStateIdxAndDistance[1] < 0)
		throw std::exception(("Did not found any open state for strobe at dmxOffset: "+std::to_string(dmxOffset)+" skipping strobe translation!").c_str());

	descPatch["strobeMap"] = ci::Json::object();
	int minValue, maxValue = -1;
	int noneValue = dmxRangeToDmxValue(extChannelDesc["capabilities"][openStateIdxAndDistance[0]]["dmxRange"]);
	descPatch["strobeMap"]["none"] = noneValue;
	
	// Check if speedStart is lower than speedEnd
	std::string str_speedStart = strobeCapability["speedStart"].get<std::string>();
	std::string str_speedEnd = strobeCapability["speedEnd"].get<std::string>();
	float speedStart, speedEnd;

	if (str_speedStart == "slow")
		speedStart = -2;
	else if (str_speedStart == "fast")
		speedStart = -1;
	else
		speedStart = speedToFloat(str_speedStart);

	if (str_speedEnd == "slow")
		speedEnd = -2;
	else if (str_speedEnd == "fast")
		speedEnd = -1;
	else
		speedEnd = speedToFloat(str_speedEnd);

	if (speedStart < speedEnd)
	{
		minValue = strobeCapability["dmxRange"][0].get<int>();
		maxValue = strobeCapability["dmxRange"][1].get<int>();

		if(speedEnd > 0)
			descPatch["strobeSpeed"] = speedEnd;
	}
	else
	{
		minValue = strobeCapability["dmxRange"][1].get<int>();
		maxValue = strobeCapability["dmxRange"][0].get<int>();

		if (speedStart > 0)
			descPatch["strobeSpeed"] = speedStart;
	}
	descPatch["strobeMap"]["min"] = minValue;
	descPatch["strobeMap"]["max"] = maxValue;

	descPatch["mapping"]["strobe"] = dmxOffset;
	std::list<std::string> notes = std::list<std::string>();
	notes.push_back("Strobe is limited to none, min and max!");
	notes.push_back("Using dmx values: no-strobe="+std::to_string(noneValue)+", min="+ std::to_string(minValue)+", max="+ std::to_string(maxValue));
	descPatch["notes"]["channel-"+std::to_string(dmxOffset)] = notes;

	return descPatch;
}
