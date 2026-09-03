
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

#include "mixer/DistanceMixer.hpp"


act::aio::DistanceMixer::DistanceMixer()
	: MixerBase()
{
}

act::aio::DistanceMixer::~DistanceMixer()
{
}

void act::aio::DistanceMixer::update()
{
	// some pseudo normalization-factor:
	calcSpeakerDistanceNorm();

	for (auto&& pair : m_mixMap) {
		auto&& sound = pair.first;
		for (auto&& speaker : m_speakers) {
			float distance = ci::distance(speaker->getPosition(), sound->getPosition()) * m_speakerDistanceNorm;
			float gain = ci::audio::decibelToLinear((1.0f - distance)*100.0f);

			if (gain < 0.0f)
				gain = 0.0f;
			else if (gain > 1.0f)
				gain = 1.0f;

			auto gainNode = pair.second[speaker->getChannel()];
			if (gainNode && gainNode->getParam()->getNumEvents() == 0)
				gainNode->getParam()->applyRamp(gain, 0.01f);
		}
	}
}

void act::aio::DistanceMixer::clear()
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
	m_mixMap = std::map<room::SoundRoomNodeRef, std::map<int, ci::audio::GainNodeRef>>();

	m_speakerDistanceNorm = 1.0f;
}

void act::aio::DistanceMixer::configure(std::vector<act::room::SpeakerRoomNodeRef> speakers, std::vector<act::room::SubwooferRoomNodeRef> subwoofers, std::vector<act::room::SoundRoomNodeRef> sounds)
{
	auto ctx = ci::audio::Context::master();
	if (!ctx->getOutput())
		return;

	clear();
	m_speakers = speakers;

	float speakerDistance = 0.0f;
	for (auto&& speaker : speakers) {
		speaker->disconnectExternals();
		for (auto&& s : speakers) {
			speakerDistance += ci::distance(speaker->getPosition(), s->getPosition());
		}
	}
	speakerDistance /= (speakers.size() * speakers.size());
	m_speakerDistanceNorm = 1.0f / (speakerDistance * 3);
	if (m_speakerDistanceNorm > 1.0f)
		m_speakerDistanceNorm = 1.0f;

	for (auto&& sound : sounds) {
		sound->disconnectExternals();
	}

	m_channelRouterNode = ctx->makeNode(new ci::audio::ChannelRouterNode(ci::audio::Node::Format().channels(ctx->getOutput()->getNumChannels())));

	for (auto&& subwoofer : subwoofers) {
		if (subwoofer->getChannel() < ctx->getOutput()->getNumChannels()) {
			subwoofer->getOut() >> m_channelRouterNode->route(0, subwoofer->getChannel()) >> ci::audio::Context::master()->getOutput();
		}
	}
	for (auto&& speaker : speakers) {
		if (speaker->getChannel() < ctx->getOutput()->getNumChannels()) {
			speaker->setupAudioNodes();
			speaker->getOut() >> m_channelRouterNode->route(0, speaker->getChannel()) >> ci::audio::Context::master()->getOutput();
		}
	}
	connectSpeakersToSubwoofers(m_speakers, subwoofers);

	for (auto&& sound : sounds) {
		connectSound(sound, speakers, subwoofers);
	}

	ctx->enable();
}

void act::aio::DistanceMixer::connectSound(act::room::SoundRoomNodeRef sound, std::vector<act::room::SpeakerRoomNodeRef> speakers, std::vector<act::room::SubwooferRoomNodeRef> subwoofers)
{
	m_mixMap[sound] = std::map<int, ci::audio::GainNodeRef>();
	auto ctx = ci::audio::Context::master();
	for (auto&& speaker : speakers) {
		if (speaker->getChannel() < ctx->getOutput()->getNumChannels()) {
			auto gain = ci::audio::Context::master()->makeNode(new ci::audio::GainNode(0.0f));
			m_mixMap[sound][speaker->getChannel()] = gain;
			sound->getOut() >> gain >> speaker->getIn();
		}
	}
}

void act::aio::DistanceMixer::calcSpeakerDistanceNorm()
{
	float speakerDistance = 0.0f;
	for (auto&& speaker : m_speakers) {
		for (auto&& s : m_speakers) {
			speakerDistance += ci::distance(speaker->getPosition(), s->getPosition());
		}
	}
	if (speakerDistance > 0.0f) {
		speakerDistance /= m_speakers.size();
		m_speakerDistanceNorm = 1.0f / (speakerDistance * 10);
		if (m_speakerDistanceNorm > 1.0f)
			m_speakerDistanceNorm = 1.0f;

	}
	else {
		m_speakerDistanceNorm = 1.0f / 10.f;
	}
}

void act::aio::DistanceMixer::connectSpeakersToSubwoofers(std::vector<act::room::SpeakerRoomNodeRef> speakers, std::vector<act::room::SubwooferRoomNodeRef> subwoofers)
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
