
/*
	InACTually
	> interactive theater for actual acts
	> this file is part of the "InACTually Engine", a MediaServer for driving all technology

	Copyright (c) 2021–2025 Lars Engeln, Fabian Töpfer
	Copyright (c) 2025 InACTually Community
	Licensed under the MIT License.
	See LICENSE file in the project root for full license information.

	This file is created and substantially modified: 2021

	contributors:
	Lars Engeln - mail@lars-engeln.de
*/

#include "mixer/NearestMixer.hpp"


act::aio::NearestMixer::NearestMixer()
	: MixerBase()
{
}

act::aio::NearestMixer::~NearestMixer()
{
}

void act::aio::NearestMixer::update()
{
	for (auto&& pair : m_mixMap) {
		auto&& sound = pair.first;

		float minDistance = FLT_MAX;
		act::room::SpeakerRoomNodeRef nearestSpeaker;
		for (auto&& speaker : m_speakers) {
			float distance = ci::distance(speaker->getPosition(), sound->getPosition());
			if (distance < minDistance) {
				minDistance = distance;
				nearestSpeaker = speaker;
			}
		}

		float gain;
		for (auto&& speaker : m_speakers) {
			if (speaker->getUID() == nearestSpeaker->getUID())
				gain = 1.0f;
			else
				gain = 0.0f;

			auto gainNode = pair.second[speaker->getUID()];
			if (gainNode && gainNode->getParam()->getNumEvents() == 0)
				gainNode->getParam()->applyRamp(gain, 0.01f);
		}
	}
}

void act::aio::NearestMixer::clear()
{
	for (auto&& speaker : m_speakers) {
		speaker->disconnectExternals();
	}
	m_speakers.clear();
	
	for (auto&& pair : m_mixMap) {
		auto&& sound = pair.first;
		sound->disconnectExternals();
	}
	m_mixMap.clear();
	m_mixMap = std::map<room::SoundRoomNodeRef, std::map<act::UID, ci::audio::GainNodeRef>>();

}

void act::aio::NearestMixer::configure(std::vector<act::room::SpeakerRoomNodeRef> speakers, std::vector<act::room::SubwooferRoomNodeRef> subwoofers, std::vector<act::room::SoundRoomNodeRef> sounds)
{
	clear();

	for (auto&& sound : sounds) {
		sound->disconnectExternals();
	}

	auto ctx = ci::audio::Context::master();
	m_channelRouterNode = ctx->makeNode(new ci::audio::ChannelRouterNode(ci::audio::Node::Format().channels(ctx->getOutput()->getNumChannels())));

	for (auto&& subwoofer : subwoofers) {
		if (subwoofer->getChannel() < ctx->getOutput()->getNumChannels()) {
			subwoofer->getOut() >> m_channelRouterNode->route(0, subwoofer->getChannel()) >> ctx->getOutput();
		}
	}
	for (auto&& speaker : speakers) {
		if (speaker->getChannel() < ctx->getOutput()->getNumChannels()) {
			speaker->getOut() >> m_channelRouterNode->route(0, speaker->getChannel()) >> ctx->getOutput();
			m_speakers.push_back(speaker);
		}
	}
	connectSpeakersToSubwoofers(m_speakers, subwoofers);

	for (auto&& sound : sounds) {
		connectSound(sound, speakers, subwoofers);
	}

	ctx->enable();
}

void act::aio::NearestMixer::connectSound(act::room::SoundRoomNodeRef sound, std::vector<act::room::SpeakerRoomNodeRef> speakers, std::vector<act::room::SubwooferRoomNodeRef> subwoofers)
{
	m_mixMap[sound] = std::map<act::UID, ci::audio::GainNodeRef>();
	auto ctx = ci::audio::Context::master();
	for (auto&& speaker : speakers) {
		if (speaker->getChannel() < ctx->getOutput()->getNumChannels()) {
			auto gain = ci::audio::Context::master()->makeNode(new ci::audio::GainNode(0.0f));
			m_mixMap[sound][speaker->getUID()] = gain;
			sound->getOut() >> gain >> speaker->getIn();
		}
	}
}

void act::aio::NearestMixer::connectSpeakersToSubwoofers(std::vector<act::room::SpeakerRoomNodeRef> speakers, std::vector<act::room::SubwooferRoomNodeRef> subwoofers)
{
	if (subwoofers.size() == 0)
		return;

	for (auto&& speaker : speakers) {
		float minDistance = FLT_MAX;
		act::room::SubwooferRoomNodeRef nearestSubwoofer;
		for (auto&& subwoofer : subwoofers) {
			float distance = ci::distance(speaker->getPosition(), subwoofer->getPosition());
			if (distance < minDistance) {
				minDistance = distance;
				nearestSubwoofer = subwoofer;
			}
		}
		speaker->getOut() >> nearestSubwoofer->getIn();
	}
}
