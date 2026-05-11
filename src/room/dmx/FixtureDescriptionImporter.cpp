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
		if (!m_oflDescriptionMapper->getLibarayPath().empty())
		{
			ImGui::Text(("Library Path: " + m_oflDescriptionMapper->getLibarayPath()).c_str());

			ImGui::Spacing();
			if (ImGui::InputText("Search OFL Fixture", m_oflFixtureFilterBuffer, IM_ARRAYSIZE(m_oflFixtureFilterBuffer)))
				m_oflFixtureFilterChanged = true;
			ImGui::Spacing();

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
					
					bool fixtureShown = false;
					for (int fixtureId = 0; fixtureId < manufacturer->fixtures.size(); fixtureId++)
					{
						OFLDescriptionMapper::OFLFixtureDescriptionRef fixture = manufacturer->fixtures.at(fixtureId);
						
						if (m_isOFLListFilteres && !fixture->showInListing)
							continue; // Skipp if we filter and fixture is not to be shown

						fixtureShown = true;
						if (ImGui::TreeNode(fixture->name.c_str()))
						{
							ImGui::Indent(1);
							if (!fixture->isDescriptionLoaded)
							{
								if (!m_oflDescriptionMapper->parseBasicDescription(fixture))
								{
									ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 0, 0, 255));
									ImGui::Text("Error while parsing the fixture description!");
									ImGui::PopStyleColor();
								}
							}

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

							if (fixture->modes.size() > fixture->selectedMode)
							{
								ImGui::SameLine();

								if (ImGui::Button("Add to project"))
								{
									m_importCallback(fixture->modes.at(fixture->selectedMode)->internalDescription);
									m_showImporter = false;
								}

								bool showMindNotes = false;

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
										if (internelParamsMapping.size() > dmxOffset && !internelParamsMapping.at(dmxOffset).internalParamName.empty())
											if (internelParamsMapping.at(dmxOffset).hasNote)
											{
												ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), internelParamsMapping.at(dmxOffset).internalParamName.c_str());
												
											
												ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "    NOTE:");
										
												for (auto const& note : fixture->modes.at(fixture->selectedMode)->internalDescription["notes"]["mapping"][dmxOffset+1])
												{
													if (!note.is_string())
														ImGui::TextColored(ImVec4(1.0f, 8.0f, 0.0f, 1.0f), "Can not display non string note, please refer to the internal fixture description!");
													else
														ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), ("    "+note.get<std::string>()).c_str());
												}
												showMindNotes = true;
											}
											else
												ImGui::Text(internelParamsMapping.at(dmxOffset).internalParamName.c_str());
										else
											ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Not Supported!");
									}

									ImGui::EndTable();

									if (showMindNotes)
										ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Yellow colored parameters have notes attached to them. They might not be fully supported!");

									if (ImGui::TreeNode("Internal fixture Description"))
									{
										ImGui::Indent(1);
										std::string const& internalDesc = fixture->modes.at(fixture->selectedMode)->internalDescription.dump(3);
										ImGui::InputTextMultiline(("Internal Description Of" + fixture->name).c_str(), const_cast<char*>(internalDesc.c_str()), internalDesc.size(), ImVec2(-FLT_MIN, 300), ImGuiInputTextFlags_ReadOnly);
										ImGui::TreePop();
										ImGui::Spacing();
									}
								}
							}
							else
								ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Could not display modes!");

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
		else
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

		}
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
