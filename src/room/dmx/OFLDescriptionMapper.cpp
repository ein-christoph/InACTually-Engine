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
	m_ofl.libraryPath = app::getAssetPath("dmx/ofl_export_ofl");
	if (!m_ofl.libraryPath.empty() && ci::fs::is_directory(m_ofl.libraryPath))
	{
		CI_LOG_I("Found Open Fixture Library:"<<m_ofl.libraryPath);
		return true;
	}
	CI_LOG_W("Did not find any open fixture library path!");
	return false;
}

std::string act::room::OFLDescriptionMapper::getLibarayPath()
{
	return m_ofl.libraryPath.string();
}

bool act::room::OFLDescriptionMapper::setLibarayPath(std::string path)
{
	m_ofl.libraryPath = path;
	//TODO: Check if location is correct
	return true;
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
	manufacturersDescriptionPath.append("manufacturers.json");
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

	fixtureDescription->isDescriptionLoaded = false;

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

		for (int i = 0; i < modeDesc["channels"].size(); i++)
		{
			const auto& modeName = modeDesc["channels"].at(i);

			if (!modeName.is_string() && !modeName.is_null())
				return false;
			oflMode.channels.push_back(modeName.is_string() ? modeName : "empty");
		}
		fixtureDescription->modes.push_back(std::make_shared<OFLMode>(oflMode));
	}

	translateOFLtoInternal(fixtureDescription);

	fixtureDescription->isDescriptionLoaded = true;

	return true;
}

// Takes a OFLFixtureDescription and tries to translate the parameters of the available modes into the inACTually internal fixture format
bool act::room::OFLDescriptionMapper::translateOFLtoInternal(OFLFixtureDescriptionRef fixtureDescription)
{
	auto const& externalDesc = fixtureDescription->externalDescription;

	for (int modeIndex = 0; modeIndex < fixtureDescription->modes.size(); modeIndex++)
	{
		OFLModeRef modeRef = fixtureDescription->modes.at(modeIndex);
		modeRef->internalDescription = ci::Json::object();

		//Fixture Name (always mode specific)
		if (!externalDesc.contains("name") || !externalDesc["name"].is_string()) 
			{ CI_LOG_E("OFL fixture import failed! No Name in external Description found."); return false; }
		std::string fixtureName = externalDesc["name"];
		modeRef->internalDescription["name"] = fixtureName + " (" + modeRef->name + " mode)";

		// Type
		// Check if it is only a dimmer
		if (externalDesc.contains("categories") && externalDesc["categories"].is_array() && externalDesc["categories"].size() == 1 && externalDesc["categories"][0] == "Dimmer")
			modeRef->internalDescription["type"] = "dimmer";
		else
			modeRef->internalDescription["type"] = "mv"; // currentry InACTually only supports dimmers and moving heads

		// Determine number of channels
		if (!externalDesc.contains("modes") || externalDesc["modes"].size() - 1 < modeIndex)
			throw std::invalid_argument("Mode not found in external Description.");
		if (!externalDesc["modes"][modeIndex].contains("channels") || !externalDesc["modes"][modeIndex]["channels"].is_array())
			throw std::invalid_argument("No ChannelArray found for selected mode.");
		modeRef->internalDescription["channel"] = externalDesc["modes"][modeIndex]["channels"].size();
		
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

			// Channels are arbitrary keys so we need to lookup type of the capability which should follow a standardised naming
			// So lets check if the channel key is in the available channels and has a capability with a type
			if (!externalDesc["availableChannels"].contains(channelKey))
			{
				CI_LOG_W("Could not find channl key '" << channelKey << "' in availableChannels! Skipping channel.");
				continue;
			}

			ci::Json internalDescPatch = ci::Json();

			if (externalDesc["availableChannels"][channelKey].contains("capability"))
			{
				try
				{
					internalDescPatch = translateChannelCapability(modeRef->internalDescription, externalDesc["availableChannels"][channelKey]["capability"], externalDesc["availableChannels"][channelKey], externalDesc, dmxOffset + 1,modeIndex);
				}
				catch (const std::exception& e)
				{
					CI_LOG_W("Could not translate capability of channel '" << channelKey << "' :" << e.what());
				}
			}
			else if (externalDesc["availableChannels"][channelKey].contains("capabilities"))
			{
				try
				{
					internalDescPatch = translateCapabilitiesChannel(modeRef->internalDescription, externalDesc["availableChannels"][channelKey]["capabilities"], externalDesc["availableChannels"][channelKey], externalDesc, dmxOffset + 1, modeIndex, channelKey);
				}
				catch (const std::exception& e)
				{
					CI_LOG_W("Could not translate capabilities of channel '" << channelKey << "' :" << e.what());
				}
			}
			else
			{
				CI_LOG_W("Channel key '" << channelKey << "' contains neither capability object or capabilities list! Skipping channel.");
				continue;
			}

			if (!internalDescPatch.is_null())
				modeRef->internalDescription.merge_patch(internalDescPatch);
			else
			{
				// If the jsonpatch is null merge_patch will clear the object which we certainly don't want to do
				// This can happen if the empty patch is initialized with ci::Json() instead of ci::Json::object()
				CI_LOG_E("Json Patch is Null! This would destroy the internalDescription and is therefore ignored! channelkey:" << channelKey);
			}

		}

		try
		{
			modeRef->internalParamsMapping = getInternalParameterMapping(modeRef->internalDescription);
		}
		catch (const std::exception& e)
		{
			CI_LOG_E("Could not build internalParamsMapping lookup! " << e.what());
		}
	}
	return true;
}

std::vector<act::room::OFLDescriptionMapper::InternalParameterInfo> act::room::OFLDescriptionMapper::getInternalParameterMapping(ci::Json internalDescription)
{
	if (!internalDescription.contains("mapping") || !internalDescription["mapping"].is_object())
		throw std::invalid_argument("internalDescription has to contain a mapping object!");
	
	if (!internalDescription.contains("channel") || !internalDescription["channel"].is_number())
		throw std::invalid_argument("internalDescription has to contain a 'channel' key with the number of channels!");
	if (internalDescription["channel"] < 0)
		throw std::invalid_argument("'channel' of internal description must not be negative!");

	int numberOfChannels = internalDescription["channel"] + 1;//Channel index starts at 1
	std::vector<InternalParameterInfo> lookup(numberOfChannels);

	for (auto const& [parameter, dmxOffset] : internalDescription["mapping"].items())
	{
		if (dmxOffset > lookup.size() - 1)
			throw std::exception("Found dmxOffset greater than number of channels!");

		if (dmxOffset.is_number()) {
			if (lookup.at(dmxOffset).internalParamName.empty())
				lookup[dmxOffset].internalParamName = parameter;
			else
				lookup.at(dmxOffset).internalParamName.append(", " + parameter);

			// Check if any note on that object exists
			lookup.at(dmxOffset).hasNote = internalDescription.contains("notes")
				&& dmxOffset < internalDescription["notes"]["mapping"].size()
				&& !internalDescription["notes"]["mapping"].at(dmxOffset.get<int>()).is_null();
		}
		else
			CI_LOG_W("Found dmxOffset which is not number in internal mapping! parameter:"<<parameter<<" dmxOffset type"<<dmxOffset.type_name()<<" Channel will be skipped in internalParameter lookup.");
	}

	if (!lookup.at(0).internalParamName.empty())
		throw std::exception("internal params mapping lookup should start at index 1 but has a value at index 0");

	lookup.erase(lookup.begin()); // Shift vector one to the left because internal channel key starts at 1 but dmxOffset starts at 0

	return lookup;
}

int act::room::OFLDescriptionMapper::convertDegStrToInt(std::string degStr, std::string decimalpoint)
{
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

int act::room::OFLDescriptionMapper::isInJsonObjArray(std::string key, std::string value, ci::Json array)
{
	if (!array.is_array()) return -1;

	bool matchKey = !key.empty();
	bool matchValue = !value.empty();

	for (int idx = 0; idx < array.size(); idx++)
	{
		for (auto const& [elemKey, elemValue] : array[idx].items())
			if ((matchKey && key == elemKey) || (matchValue && value == elemValue)) return idx;
	}

	return -1;
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

ci::Json act::room::OFLDescriptionMapper::addMappingNotesToPatch(int dmxOffset, std::list<std::string>& notes, ci::Json const& internalDesc)
{
	ci::Json descPatch = ci::Json::object();
	if (internalDesc.contains("notes") && internalDesc["notes"].is_object()
		&& internalDesc["notes"].contains("mapping"))
	{
		if (!internalDesc["notes"]["mapping"].is_array())
			throw std::invalid_argument("mapping of notes is not an array!");

		descPatch["mapping"] = internalDesc["notes"]["mapping"];
	}
	else
		descPatch["mapping"] = ci::Json::array();

	if (dmxOffset < descPatch["mapping"].size() && !descPatch["mapping"][dmxOffset].is_null())
		descPatch["mapping"][dmxOffset].push_back(notes);
	else
		descPatch["mapping"][dmxOffset] = notes;

	return descPatch;
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

ci::Json act::room::OFLDescriptionMapper::translateChannelCapability(ci::Json const& internalDesc, ci::Json const& extCapabilityDesc, ci::Json const& extChannelDesc, ci::Json const& fullExtDesc, int dmxOffset, int modeIndex)
{
	if (!extCapabilityDesc.contains("type"))
		throw std::invalid_argument("external capability description does not include 'type' key!");
	if (!extCapabilityDesc["type"].is_string())
		throw std::invalid_argument("type of external capability description is not a string!");

	std::string const& capabilityType = extCapabilityDesc["type"];

	//For a list of current capability types see https://github.com/OpenLightingProject/open-fixture-library/blob/master/docs/capability-types.md
	try
	{
		if (capabilityType == "NoFunction")
			return ci::Json::object(); // NoFunction can be ignored except for some special cases which should not matter for our purpose
		else if (capabilityType == "Intensity")
			return translateIntensityCapability(internalDesc, extCapabilityDesc, extChannelDesc, fullExtDesc, dmxOffset, modeIndex);
		else if (capabilityType == "Pan")
			return translatePanCapability(internalDesc, extCapabilityDesc, extChannelDesc, fullExtDesc, dmxOffset, modeIndex);
		else if (capabilityType == "Tilt")
			return translateTiltCapability(internalDesc, extCapabilityDesc, extChannelDesc, fullExtDesc, dmxOffset, modeIndex);
		else if (capabilityType == "Zoom")
			return translateZoomCapability(internalDesc, extCapabilityDesc, extChannelDesc, fullExtDesc, dmxOffset, modeIndex);
		else if (capabilityType == "ColorIntensity")
			return translateColorIntensityCapability(internalDesc, extCapabilityDesc, extChannelDesc, fullExtDesc, dmxOffset, modeIndex);
		else if(capabilityType == "PanTiltSpeed")
			return translatePanTiltSpeedCapability(internalDesc, extCapabilityDesc, extChannelDesc, fullExtDesc, dmxOffset, modeIndex);
		else
			CI_LOG_W("Can not translate capability of type '" << capabilityType << "' because no mapping exists!");
	}
	catch (const std::exception& e)
	{
		CI_LOG_W("Could not translate capability of type '" << capabilityType << "'! " << e.what() << " Skipping capability translation.");
	}

	return ci::Json::object();
}

ci::Json act::room::OFLDescriptionMapper::translateCapabilitiesChannel(ci::Json const& internalDesc, ci::Json const& extCapabilitiesDesc, ci::Json const& extChannelDesc, ci::Json const& fullExtDesc, int dmxOffset, int modeIndex, std::string const& channelName)
{
	if (isColorWheel(channelName, extChannelDesc, fullExtDesc, dmxOffset, modeIndex))
		return translateColorWheelCapability(internalDesc, extCapabilitiesDesc, extChannelDesc, fullExtDesc, dmxOffset, modeIndex, channelName);
	else if (isShutterStrobeCapability(channelName, extChannelDesc, fullExtDesc, dmxOffset, modeIndex))
		return translateShutterStrobeCapability(internalDesc, extCapabilitiesDesc, extChannelDesc, fullExtDesc, dmxOffset, modeIndex, channelName);
	else
		CI_LOG_W("Can not translate capabilities of channel '" << channelName << "' because no mapping exists!");

	return ci::Json();
}

ci::Json act::room::OFLDescriptionMapper::translateIntensityCapability(ci::Json const& internalDesc, ci::Json const& extCapabilityDesc, ci::Json const& extChannelDesc, ci::Json const& fullExtDesc, int dmxOffset, int modeIndex)
{
	ci::Json descPatch = ci::Json::object();

	if (internalDesc.contains("mapping") && internalDesc["mapping"].contains("dimmer"))
	{
		// We can only handle one dimmer
		throw std::exception("Dimmer already defined in mapping");
	}

	descPatch["mapping"]["dimmer"] = dmxOffset;

	auto const& fineChannelMap = resolveFineChannels(fullExtDesc, modeIndex, extChannelDesc, 1);
	if (fineChannelMap.size() == 1 && fineChannelMap.begin()->second >= 0)
		descPatch["mapping"]["fineDimmer"] = fineChannelMap.begin()->second + 1; //Add +1 because inACTually starts at 1 but channels index starts at 0

	return descPatch;
}

ci::Json act::room::OFLDescriptionMapper::translatePanCapability(ci::Json const& internalDesc, ci::Json const& extCapabilityDesc, ci::Json const& extChannelDesc, ci::Json const& fullExtDesc, int dmxOffset, int modeIndex)
{
	ci::Json descPatch = ci::Json::object();

	if (internalDesc.contains("mapping") && internalDesc["mapping"].contains("pan"))
		throw std::exception("Pan already defined in mapping!"); // Currently we can only handle one pan per fixture

	// Check angle start and angle end to calculate panRange
	if (!extCapabilityDesc.contains("angleStart") || !extCapabilityDesc["angleStart"].is_string()) 
		throw std::exception("No angleStart in extCapabilityDesc!");
	int angleStart = convertDegStrToInt(extCapabilityDesc["angleStart"]);

	if (!extCapabilityDesc.contains("angleEnd") || !extCapabilityDesc["angleEnd"].is_string())
		throw std::exception("No angleEnd in extCapabilityDesc!");
	int angleEnd = convertDegStrToInt(extCapabilityDesc["angleEnd"]);

	if (angleEnd < angleStart) throw std::exception("angleStart is larger that angleEnd in pan capability!");

	descPatch["panRange"] = angleEnd - angleStart;

	descPatch["mapping"]["pan"] = dmxOffset;

	auto const& fineChannelMap = resolveFineChannels(fullExtDesc, modeIndex, extChannelDesc, 1);
	if (fineChannelMap.size() == 1 && fineChannelMap.begin()->second >= 0)
		descPatch["mapping"]["finePan"] = fineChannelMap.begin()->second + 1; //Add +1 because inACTually starts at 1 but channels index starts at 0

	return descPatch;
}

ci::Json act::room::OFLDescriptionMapper::translateTiltCapability(ci::Json const& internalDesc, ci::Json const& extCapabilityDesc, ci::Json const& extChannelDesc, ci::Json const& fullExtDesc, int dmxOffset, int modeIndex)
{
	ci::Json descPatch = ci::Json::object();

	if (internalDesc.contains("mapping") && internalDesc["mapping"].contains("tilt"))
		throw std::exception("Tilt already defined in mapping!"); // Currently we can only handle one tilt per fixture

	// Check angle start and angle end to calculate tiltRange
	if (!extCapabilityDesc.contains("angleStart") || !extCapabilityDesc["angleStart"].is_string())
		throw std::exception("No angleStart in extCapabilityDesc!");
	int angleStart = convertDegStrToInt(extCapabilityDesc["angleStart"]);

	if (!extCapabilityDesc.contains("angleEnd") || !extCapabilityDesc["angleEnd"].is_string())
		throw std::exception("No angleEnd in extCapabilityDesc!");
	int angleEnd = convertDegStrToInt(extCapabilityDesc["angleEnd"]);

	if (angleEnd < angleStart) throw std::exception("angleStart is larger that angleEnd in tilt capability!");

	descPatch["tiltRange"] = angleEnd - angleStart;

	descPatch["mapping"]["tilt"] = dmxOffset;

	auto const& fineChannelMap = resolveFineChannels(fullExtDesc, modeIndex, extChannelDesc, 1);
	if (fineChannelMap.size() == 1 && fineChannelMap.begin()->second >= 0)
		descPatch["mapping"]["fineTilt"] = fineChannelMap.begin()->second + 1; //Add +1 because inACTually starts at 1 but channels index starts at 0

	return descPatch;
}

ci::Json act::room::OFLDescriptionMapper::translateZoomCapability(ci::Json const& internalDesc, ci::Json const& extCapabilityDesc, ci::Json const& extChannelDesc, ci::Json const& fullExtDesc, int dmxOffset, int modeIndex)
{
	ci::Json descPatch = ci::Json::object();

	if (internalDesc.contains("mapping") && internalDesc["mapping"].contains("zoom"))
		throw std::invalid_argument("zoom already defined in mapping!");

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

ci::Json act::room::OFLDescriptionMapper::translateColorIntensityCapability(ci::Json const& internalDesc, ci::Json const& extCapabilityDesc, ci::Json const& extChannelDesc, ci::Json const& fullExtDesc, int dmxOffset, int modeIndex)
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

ci::Json act::room::OFLDescriptionMapper::translatePanTiltSpeedCapability(ci::Json const& internalDesc, ci::Json const& extCapabilityDesc, ci::Json const& extChannelDesc, ci::Json const& fullExtDesc, int dmxOffset, int modeIndex)
{
	ci::Json descPatch = ci::Json::object();
	
	if (internalDesc.contains("mapping") && internalDesc["mapping"].contains("speed"))
		throw std::exception("speed already defined in mapping, skipping PanTilTSpeed!");
	
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

	// WheelName can be channel name or stated explicitly in the description
	std::string WheelName = (extChannelDesc["capabilities"][idx].contains("wheel") && extChannelDesc["capabilities"][idx]["wheel"].is_string()) ? extChannelDesc["capabilities"][idx]["wheel"].get<std::string>() : channelName;

	if (!fullExtDesc.contains("wheels") || !fullExtDesc["wheels"].contains(WheelName) || !fullExtDesc["wheels"][WheelName].contains("slots") || !fullExtDesc["wheels"][WheelName]["slots"].is_array())
		return false; // Color wheels always have a seperate description holding the information about the colors

	if(isInJsonObjArray("type", "Color", fullExtDesc["wheels"][WheelName]["slots"]) < 0)
		return false; // if no slots of the wheel holds color it is not a color wheel
	
	return true; // Otherwise it is a color wheel
}

ci::Json act::room::OFLDescriptionMapper::translateColorWheelCapability(ci::Json const& internalDesc, ci::Json const& extCapabilityDesc, ci::Json const& extChannelDesc, ci::Json const& fullExtDesc, int dmxOffset, int modeIndex, std::string const& channelName)
{

	if (!extChannelDesc.contains("capabilities") ||  !extChannelDesc["capabilities"].is_array())
		throw std::invalid_argument("A Color Wheeel has to have an array of WheelSlot capabilities");

	if (!fullExtDesc.contains("wheels") || !fullExtDesc["wheels"].is_object())
		throw std::invalid_argument("For a Color Wheel a wheels object has to be present in the external description");

	if (internalDesc.contains("colorMap") || (internalDesc.contains("mapping") && internalDesc["mapping"].contains("color")))
		throw std::invalid_argument("internalDesc already contains definition for a color wheel. Currently only one color wheel is supported.");

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
			CI_LOG_W("Color Wheel translation WheelSlot at position " << idx << " does not specify slotNumbers! Half frames currently not supported, skipping slot.");
			notes.push_back("Wheel slot " + idx + " unavailable, no slot number provided!");
			continue;
		}

		int slotIdx = capability["slotNumber"] - 1; //SlotNumber starts at 1 but is referencing an array

		if (!fullExtDesc["wheels"][WheelName].is_object() || !fullExtDesc["wheels"][WheelName].contains("slots") || !fullExtDesc["wheels"][WheelName]["slots"].is_array())
			throw std::invalid_argument("Malformed wheels wheel '" + WheelName + "'");

		if (slotIdx >= fullExtDesc["wheels"][WheelName]["slots"].size())
			throw std::invalid_argument("Channel '" + channelName + "' specifies slotNumber greater that available slots in wheels description!");

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

	descPatch["notes"] = addMappingNotesToPatch(dmxOffset, notes, internalDesc);

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

ci::Json act::room::OFLDescriptionMapper::translateShutterStrobeCapability(ci::Json const& internalDesc, ci::Json const& extCapabilityDesc, ci::Json const& extChannelDesc, ci::Json const& fullExtDesc, int dmxOffset, int modeIndex, std::string const& channelName)
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
	{
		CI_LOG_W("encountered incompatible strobe capabilities list at dmxOffset " << dmxOffset);
		return descPatch;
	}

	// find neares open position
	ci::Json const& strobeCapability = extChannelDesc["capabilities"][strobeIdx];
	if (!strobeCapability.contains("dmxRange") || !strobeCapability["dmxRange"].is_array() || strobeCapability["dmxRange"].size() != 2)
	{
		CI_LOG_W("Could not find well formatted dmxRange in strobe capability!");
		return descPatch;
	}

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
	{
		CI_LOG_W("Did not found any open state for strobe at dmxOffset: "<<dmxOffset<<" skipping strobe translation!");
		return descPatch;
	}

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
	descPatch["notes"] = addMappingNotesToPatch(dmxOffset, notes, internalDesc);


	return descPatch;
}
