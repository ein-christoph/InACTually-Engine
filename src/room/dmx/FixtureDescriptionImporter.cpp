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

/* The FixtureDescriptionImporter translates external fixture descriptions into the InACTually internal description format.
*  Description mappers are used to do the translation of a specific external format into ours.
*/

#include "roompch.hpp"
#include "dmx/FixtureDescriptionImporter.hpp"

act::room::FixtureDescriptionImporter::FixtureDescriptionImporter(std::function<void(ci::Json)> importCallback)
{
	m_importCallback = importCallback;

	m_oflDescriptionMapper = OFLDescriptionMapper::create();
	m_showImporter = false;
}

act::room::FixtureDescriptionImporter::~FixtureDescriptionImporter()
{
}

void act::room::FixtureDescriptionImporter::draw()
{
	if (ImGui::Button("Import Fixture Description"))
		m_showImporter = true;

	if (!m_showImporter)
		return;

	ImGui::OpenPopup("Fixture Import");

	if (ImGui::BeginPopupModal("Fixture Import"))
	{
		
		drawOFLImport();

		if (ImGui::Button("Close Importer"))
		{
			m_showImporter = false;
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}
}

void act::room::FixtureDescriptionImporter::update()
{
	if (m_openOFLInBrowser)
	{
		CI_LOG_D("Opening Open Fixture Libaray Website");
		ci::app::Platform::get()->launchWebBrowser(Url("https://open-fixture-library.org/"));
		m_openOFLInBrowser = false;
	}
	if (m_searchOFLAgain)
	{
		CI_LOG_D("Trying to find Open Fixture Library again...");
		m_oflDescriptionMapper->searchLibraryPath();
		m_searchOFLAgain = false;
	}
	if (m_oflFixtureFilterChanged)
	{
		filterOFLFixtures();
		m_oflFixtureFilterChanged = false;
	}
}

void act::room::FixtureDescriptionImporter::drawOFLImport()
{
	if (ImGui::CollapsingHeader(m_oflDescriptionMapper->getName().c_str()))
	{
		// Check if the ofl library is present
		if (m_oflDescriptionMapper->getLibarayPath().empty())
		{
			ImGui::Text("No Path to the Open Fixture Libaray Found!");
			ImGui::Spacing();
			ImGui::TextWrapped("Please download the 'Open Fixture Libaray JSON' ZIP-Archive from https://open-fixture-library.org/ and extract it as 'ofl_export_ofl' into the 'dmx' subfolder of the assets folder.");
			std::string assetsPath = "Current assets folder: " + getAssetPath("").string();
			ImGui::Spacing();
			ImGui::TextWrapped(assetsPath.c_str());
			if (ImGui::Button("Open OFL Website"))
				m_openOFLInBrowser = true;
			ImGui::SameLine();
			if (ImGui::Button("Search Again"))
				m_searchOFLAgain = true;
			ImGui::Spacing();
			return;
		}

		//== Header with library path an search box
		ImGui::Text(("Library Path: " + m_oflDescriptionMapper->getLibarayPath().string()).c_str());

		ImGui::Spacing();
		if (ImGui::InputText("Search OFL Fixture", m_oflFixtureFilterBuffer, IM_ARRAYSIZE(m_oflFixtureFilterBuffer)))
			m_oflFixtureFilterChanged = true;
		ImGui::Spacing();

		//== Loop over all manufactueres
		std::vector<act::room::OFLDescriptionMapper::OFLManufacturerRef> manufacturers = m_oflDescriptionMapper->getManufacturers(true);
		for (int manufacturerId = 0; manufacturerId < manufacturers.size(); manufacturerId++)
		{
			OFLDescriptionMapper::OFLManufacturerRef manufacturer = manufacturers.at(manufacturerId);
				
			if (m_isOFLListFilteres && !manufacturer->showInListing)
				continue; // Skipp if we filter and manufacturer is not to be shown


			if (m_isOFLListFilteres && manufacturer->expandInListing)
				ImGui::SetNextItemOpen(true);

			if (ImGui::TreeNode(manufacturer->name.c_str()))
			{
				ImGui::Indent(1);
				
				//== And all fixtures to display them in a tree hirarchy
				bool fixtureShown = false;
				for (int fixtureId = 0; fixtureId < manufacturer->fixtures.size(); fixtureId++)
				{
					OFLDescriptionMapper::OFLFixtureDescriptionRef fixture = manufacturer->fixtures.at(fixtureId);

					if (m_isOFLListFilteres && !fixture->showInListing)
						continue; // Skipp if we filter and fixture is not to be shown
						
					fixtureShown = true;

					//== Draw the details of the fixture including mode selector and import button
					if (ImGui::TreeNode(fixture->name.c_str()))
					{
						ImGui::Indent(1);
						drawOFLFixtureDetails(fixture, manufacturerId, fixtureId);
						ImGui::TreePop();
						ImGui::Spacing();
					}
				}
				
				if (!fixtureShown)
					ImGui::Text("No fixture matching the search term found!");

				ImGui::TreePop();
				ImGui::Spacing();
			}
		}
	}
}

void act::room::FixtureDescriptionImporter::drawOFLFixtureDetails(OFLDescriptionMapper::OFLFixtureDescriptionRef fixture, int manufacturerId, int fixtureId)
{

	//== Check if fixture description was sucessfully translated
	if (!fixture->isDescriptionLoaded && !m_oflDescriptionMapper->parseBasicDescription(fixture))
	{
		ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 0, 0, 255));
		ImGui::Text("Error while parsing the fixture description!");
		ImGui::PopStyleColor();
	}

	//== Mode selection combo box
	const char* preview_val = fixture->selectedMode >= 0 && fixture->selectedMode < fixture->modes.size()
		? fixture->modes.at(fixture->selectedMode)->name.c_str() : "Select Mode...";

	if (ImGui::BeginCombo("Mode", preview_val))
	{
		for (int i = 0; i < fixture->modes.size(); i++)
		{
			if (ImGui::Selectable(fixture->modes.at(i)->name.c_str(), fixture->selectedMode == i))
				fixture->selectedMode = i;

			if (fixture->selectedMode == i)
				ImGui::SetItemDefaultFocus();

		}
		ImGui::EndCombo();
	}

	if (fixture->selectedMode >= fixture->modes.size())
	{
		ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Can not display selected mode!");
		return;
	}
	
	//== Table for displaying the translation
	drawOFLFixtureTranslationTable(fixture, manufacturerId, fixtureId);

	ImGui::Spacing();
	ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "Not all OFL fixtures are fully supported. The automatic translation tries to match the ofl description as best as possible to the InACTually internal description.");
	
	//== Show internal description in readonly textbox
	if (ImGui::TreeNode("Internal fixture Description"))
	{
		ImGui::Indent(1);

		// store json dump in map so the inputTextMultline callback has something to work with
		std::string internalDescKey = std::to_string(manufacturerId) + std::to_string(fixtureId) + std::to_string(fixture->selectedMode);

		if (m_oflTranslationJsonDmpCache.find(internalDescKey) == m_oflTranslationJsonDmpCache.end())
		{
			std::string newInternalDescDmp = fixture->modes.at(fixture->selectedMode)->internalDescription.dump(3);
			m_oflTranslationJsonDmpCache.insert({ internalDescKey, newInternalDescDmp });
		}
		ImGui::InputTextMultiline(internalDescKey.c_str(), &m_oflTranslationJsonDmpCache.at(internalDescKey), ImVec2(-FLT_MIN, 300), ImGuiInputTextFlags_ReadOnly);

		ImGui::TreePop();
		ImGui::Spacing();
	}

	// === Import Button
	ImGui::TextColored(ImVec4(1.0f, 0.7f, 0.0f, 1.0f), "Always check plausibility of the automatic translation before importing the fixture!");
			
	if (ImGui::Button("Add Fixture To Project"))
	{
		m_importCallback(fixture->modes.at(fixture->selectedMode)->internalDescription);
		m_showImporter = false;
	}
}

void act::room::FixtureDescriptionImporter::drawOFLFixtureTranslationTable(OFLDescriptionMapper::OFLFixtureDescriptionRef fixture, int manufacturerId, int fixtureId)
{
	bool showMindNotes = false;

	//== Table for displaying the translation
	if (ImGui::BeginTable((fixture->name + "Mode Details").c_str(), 3, ImGuiTableFlags_Borders | ImGuiTableFlags_Resizable | ImGuiTableFlags_RowBg))
	{

		ImGui::TableSetupColumn("Channel");
		ImGui::TableSetupColumn("OFL Channel Name");
		ImGui::TableSetupColumn("InACTually Parameter Name");
		ImGui::TableHeadersRow();

		std::vector<act::room::OFLDescriptionMapper::InternalParameterInfo> const& internelParamsMapping = fixture->modes.at(fixture->selectedMode)->internalParamsMapping;

		for (int dmxOffset = 0; dmxOffset < fixture->modes.at(fixture->selectedMode)->channels.size(); dmxOffset++)
		{
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("%i", dmxOffset + 1);
			ImGui::TableSetColumnIndex(1);
			ImGui::Text(fixture->modes.at(fixture->selectedMode)->channels.at(dmxOffset).c_str());
			ImGui::TableSetColumnIndex(2);

			if (internelParamsMapping.size() <= dmxOffset || internelParamsMapping.at(dmxOffset).internalParamName.empty())
			{
				ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Not Supported!");
				continue;
			}

			if (!internelParamsMapping.at(dmxOffset).hasNote)
			{
				ImGui::Text(internelParamsMapping.at(dmxOffset).internalParamName.c_str());
				continue;
			}

			// The parameter is supported but has notes atached so we should display them to the user
			ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), internelParamsMapping.at(dmxOffset).internalParamName.c_str());


			ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "    NOTE:");

			for (auto const& note : fixture->modes.at(fixture->selectedMode)->internalDescription["notes"]["mapping"][dmxOffset + 1])
			{
				if (!note.is_string())
					ImGui::TextColored(ImVec4(1.0f, 8.0f, 0.0f, 1.0f), "Can not display non string note, please refer to the internal fixture description!");
				else
					ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), ("    " + note.get<std::string>()).c_str());
			}
			showMindNotes = true;
		}

		ImGui::EndTable();

		if (showMindNotes)
			ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Yellow colored parameters have notes attached to them. They might not be fully supported!");
	}
}

void act::room::FixtureDescriptionImporter::filterOFLFixtures()
{
	std::string filterTerm(m_oflFixtureFilterBuffer);
	std::transform(filterTerm.begin(), filterTerm.end(), filterTerm.begin(), [](unsigned char c) {return std::tolower(c);});
	if (filterTerm.size() < 3)
	{
		// For performance reasons just filter if the search string has at least three characters
		m_isOFLListFilteres = false;
		return;
	}

	m_isOFLListFilteres = true;

	for (auto const& manufacturer : m_oflDescriptionMapper->getManufacturers(false))
	{
		bool fixtureWithText = false;

		std::string manufacturerLowerCase = manufacturer->name;
		std::transform(manufacturerLowerCase.begin(), manufacturerLowerCase.end(), manufacturerLowerCase.begin(), [](unsigned char c) {return std::tolower(c);});
		
		bool forceShowFixtures = manufacturerLowerCase == filterTerm; // Show all Fixtures if manufacturer name matches search term exactly
		
		for (auto const& fixture : manufacturer->fixtures)
		{
			if (fixture->name.find(filterTerm) != std::string::npos || forceShowFixtures)
			{
				fixtureWithText = true;
				fixture->showInListing = true;
			}
			else
				fixture->showInListing = false;
		}

		manufacturer->expandInListing = fixtureWithText;

		if (fixtureWithText || manufacturerLowerCase.find(filterTerm) != std::string::npos)
			manufacturer->showInListing = true;
		else
			manufacturer->showInListing = false;
	}
}
