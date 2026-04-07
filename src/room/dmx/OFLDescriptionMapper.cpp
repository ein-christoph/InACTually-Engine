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

			if (externalDesc["availableChannels"][channelKey].contains("capability"))
			{
				try
				{
					ci::Json const& internalDescPatch = translateChannelCapability(modeRef->internalDescription, externalDesc["availableChannels"][channelKey]["capability"], externalDesc["availableChannels"][channelKey], externalDesc, dmxOffset + 1,modeIndex);
					if (!internalDescPatch.is_null())
						modeRef->internalDescription.merge_patch(internalDescPatch);
					else
					{
						// If the jsonpatch is null merge_patch will clear the object which we certainly don't want to do
						// This can happen if the empty patch is initialized with ci::Json() instead of ci::Json::object()
						CI_LOG_E("Json Patch is Null! This would destroy the internalDescription and is therefore ignored! channelkey:" << channelKey);
					}
				}
				catch (const std::exception& e)
				{
					CI_LOG_W("Could not translate capability of channel '" << channelKey << "' :" << e.what());
				}
			}
			else if (externalDesc["availableChannels"][channelKey].contains("capabilities"))
			{
				CI_LOG_W("Channel key '" << channelKey << "' contains list of capabilities! Capabilitie lists are not yet supported, skipping channel.");
				continue;
			}
			else
			{
				CI_LOG_W("Channel key '" << channelKey << "' contains neither capability object or capabilities list! Skipping channel.");
				continue;
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

std::vector<std::string> act::room::OFLDescriptionMapper::getInternalParameterMapping(ci::Json internalDescription)
{
	if (!internalDescription.contains("mapping") || !internalDescription["mapping"].is_object())
		throw std::invalid_argument("internalDescription has to contain a mapping object!");
	
	if (!internalDescription.contains("channel") || !internalDescription["channel"].is_number())
		throw std::invalid_argument("internalDescription has to contain a 'channel' key with the number of channels!");
	if (internalDescription["channel"] < 0)
		throw std::invalid_argument("'channel' of internal description must not be negative!");

	int numberOfChannels = internalDescription["channel"] + 1;//Channel index starts at 1
	std::vector<std::string> lookup(numberOfChannels);

	for (auto const& [parameter, dmxOffset] : internalDescription["mapping"].items())
	{
		if (dmxOffset > lookup.size() - 1)
			throw std::exception("Found dmxOffset greater than number of channels!");

		if (dmxOffset.is_number())
			if (lookup.at(dmxOffset).empty())
				lookup[dmxOffset] = parameter;
			else
				lookup.at(dmxOffset).append(", " + parameter);
		else
			CI_LOG_W("Found dmxOffset which is not number in internal mapping! parameter:"<<parameter<<" dmxOffset type"<<dmxOffset.type_name()<<" Channel will be skipped in internalParameter lookup.");
	}

	if (!lookup.at(0).empty())
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
		else
			CI_LOG_W("Can not translate capability of type '" << capabilityType << "' because no mapping exists!");
	}
	catch (const std::exception& e)
	{
		CI_LOG_W("Could not translate capability of type '" << capabilityType << "'! " << e.what() << " Skipping capability translation.");
	}

	return ci::Json::object();
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
