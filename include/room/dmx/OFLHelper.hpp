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

	The OFLHelper.hpp contains a collection of static methods used to handle the Open Fixture Library
*/
#pragma once
#include "roompch.hpp"


namespace act {
	namespace room {
		class OFLHelper
		{
		public: 

			// Adds a note at internalDescPatch["notes"]["channel-<dmxOffsett>"]
			static void attachNoteToChannelDesc(ci::Json& internalDescPatch, int dmxOffset, std::string note)
			{
				if (!internalDescPatch.contains("notes"))
					internalDescPatch["notes"] = ci::Json::object();

				std::string channelKey = "channel-" + std::to_string(dmxOffset);

				if (internalDescPatch["notes"].contains(channelKey) && !internalDescPatch["notes"][channelKey].is_array())
				{
					// There is already a notes list for that channel but it is not an array
					CI_LOG_E("could not attach note to channel " << channelKey << " because there are notes but not as array! note:" << note);
					return;
				}
				else
				{
					// There is no notes list jet so we create an array
					internalDescPatch["notes"][channelKey] = ci::Json::array();
				}
				internalDescPatch["notes"][channelKey].push_back(note);
			};

			// Checks if there is a wheelName specified for the capabilities list, returns the channelname otherwise
			// WheelName can be channel name or stated explicitly in the description
			static std::string getWheelNameKey(std::string const& channelName, ci::Json const& extCapabilities)
			{
				return (extCapabilities.contains("wheel") && extCapabilities["wheel"].is_string()) ? extCapabilities["wheel"].get<std::string>() : channelName;
			};

			// Converts a degree string into an integer
			// NOTE: angles in OFL can also be in % but since inACTually currently can not handle 
			// proportional angles it wont be converted but result in an exception
			static int convertDegStrToInt(std::string degStr, std::string decimalpoint = ".")
			{
				size_t degPos = degStr.find("deg");

				if (degPos == std::string::npos) throw std::invalid_argument("degStr does not contain 'deg'!");
				if (degStr.length() - degPos != 3) throw std::invalid_argument("degStr does not contain 'deg' as last three characters!");

				degStr.erase(degStr.length() - 3, 3);

				if (degStr.find(decimalpoint) != std::string::npos)
					return static_cast<int>(std::round(std::stof(degStr)));
				else
					return std::stoi(degStr);
			};

			// converts an ofl speed string into a float
			static float speedToFloat(std::string speed)
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
			};

			// Calculates the distance in address offset from baseRange to distantRange
			static int dmxRangeDistance(ci::Json const& distantRange, ci::Json const& baseRange)
			{
				if (!distantRange.is_array() || !baseRange.is_array()
					|| distantRange.size() != 2 || baseRange.size() != 2)
					throw new std::invalid_argument("Malformed dmxRanges!");

				if (distantRange[1] < baseRange[0])
					return distantRange[1].get<int>() - baseRange[0].get<int>(); // distantRange below baseRange
				if (baseRange[1] < distantRange[0])
					return distantRange[0].get<int>() - baseRange[1].get<int>(); // disntantRange above baseRange 

				return 0; // distantRange within baseRange (/or overlapping)
			};

			// Averages a dmx range to one dmx value
			static int dmxRangeToDmxValue(ci::Json const& dmxRange)
			{
				if (!dmxRange.is_array() || dmxRange.size() != 2)
					throw std::invalid_argument("Malformed dmxRange!");

				return std::round((dmxRange[0].get<int>() + dmxRange[1].get<int>()) * 0.5);
			};
		};
	}
}