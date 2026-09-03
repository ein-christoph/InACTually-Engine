
/*
	InACTually
	> interactive theater for actual acts
	> this file is part of the "InACTually Engine", a MediaServer for driving all technology

	Copyright (c) 2021–2025 Lars Engeln, Fabian Töpfer
	Copyright (c) 2025 InACTually Community
	Licensed under the MIT License.
	See LICENSE file in the project root for full license information.

	This file is created and substantially modified: 2023

	contributors:
	Lars Engeln - mail@lars-engeln.de
*/

#include "TimeStretchingNode.hpp"
#include "cinder/app/App.h"


using namespace std;

#include "cinder/audio/audio.h"

void act::aio::TimeStretchingNode::initialize() {
	m_fileSampleRate = m_sourceFile->getSampleRate();
	m_buffer = m_sourceFile->loadBuffer();
	m_currentPlayPosition = 0;
	// Calculate sample rate adjustment for playback speed
	m_sampleRateAdjustment = static_cast<float>(m_sourceFile->getSampleRate()) / static_cast<float>(getSampleRate());
	m_speed = m_sampleRateAdjustment;
	m_pitch = 1.0f;
	// Allocate buffers for FFT and speed modulation
	m_fftBuffer = new ci::audio::Buffer(m_fftBufferSize, 1);
	m_speedModulationBuffer = new ci::audio::Buffer(static_cast<size_t>(m_fftBufferSize * (1.0f / m_minSpeed)), 1);
	// Create ring buffer for audio output
	m_ringBuffer = std::make_shared<ci::audio::dsp::RingBufferT<float>>(static_cast<size_t>(m_fftBufferSize * 2 / m_minSpeed));
	// Start timestretching thread
	m_thread = std::make_shared<std::thread>(&TimeStretchingNode::timestretching, this);
}

// Destructor: ensures thread is joined before destruction
act::aio::TimeStretchingNode::~TimeStretchingNode() {
	if (m_thread && m_thread->joinable()) {
		m_thread->join();
	}
}

// Sets playback speed and updates pitch accordingly
void act::aio::TimeStretchingNode::setPlaybackSpeed(float speed) {
	m_speed = m_sampleRateAdjustment * speed;
	m_speed = std::clamp(m_speed.load(), m_minSpeed, m_maxSpeed);
	m_pitch = m_sampleRateAdjustment / m_speed;
}

double act::aio::TimeStretchingNode::calcBPM() {
	// TODO
	return m_bpm;
}

// Loads a new track and sets BPM if provided
double act::aio::TimeStretchingNode::setNewTrack(std::string assetName, double bpm) {
	m_isRequestingStop = true;
	volatile int busy = 0;
	// Wait until node is idle before loading new track
	while (!m_isIdle) {
		if (busy > 0)
			std::cout << "this just here to prevent the compiler from optimizing the loop away";
	}
	m_sourceFile = ci::audio::load(ci::app::loadAsset(assetName));
	if (bpm == -1) {
		setNewSourceFile(m_sourceFile);
	} else {
		setNewSourceFileCashedBPM(m_sourceFile, bpm);
	}
	return m_bpm;
}

float act::aio::TimeStretchingNode::getPlaybackSpeed() {
	return m_speed / m_sampleRateAdjustment;
}

bool act::aio::TimeStretchingNode::getUsePitchCorrection() {
	return m_usePitchCorrection;
}

// Empties the ring buffer by reading all available samples
void act::aio::TimeStretchingNode::emptyRingBuffer() {
	size_t obsoleteSamples = m_ringBuffer->getAvailableRead();
	auto bufferDump = std::make_unique<ci::audio::Buffer>(obsoleteSamples, 1);
	m_ringBuffer->read(bufferDump->getChannel(0), obsoleteSamples);
}

// Sets a new source file and recalculates parameters
void act::aio::TimeStretchingNode::setNewSourceFile(ci::audio::SourceFileRef sf) {
	m_sourceFile = sf;
	m_buffer = m_sourceFile->loadBuffer();
	m_bpm = calcBPM();
	m_sampleRateAdjustment = static_cast<float>(m_sourceFile->getSampleRate()) / static_cast<float>(getSampleRate());
	m_speed = m_sampleRateAdjustment;
	m_pitch = 1.0f;
	m_isRequestingStop = false;
}

// Sets a new source file and uses provided BPM
void act::aio::TimeStretchingNode::setNewSourceFileCashedBPM(ci::audio::SourceFileRef sf, double BPM) {
	m_sourceFile = sf;
	m_buffer = m_sourceFile->loadBuffer();
	m_bpm = BPM;
	m_sampleRateAdjustment = static_cast<float>(m_sourceFile->getSampleRate()) / static_cast<float>(getSampleRate());
	m_speed = m_sampleRateAdjustment;
	m_pitch = 1.0f;
	m_isRequestingStop = false;
}

void act::aio::TimeStretchingNode::togglePitchCorrection() {
	m_usePitchCorrection = !m_usePitchCorrection;
}

// Main timestretching loop running in a separate thread
void act::aio::TimeStretchingNode::timestretching() {
	auto* fileData = m_buffer->getData();
	auto* FftData = m_fftBuffer->getData();
	auto* OutData = m_speedModulationBuffer->getData();
	bool isFinished = false;
	while (true) {
		// If stop requested, reset playback position
		if (m_isRequestingStop) {
			*fileData = 0;
			m_currentPlayPosition = 0;
		}
		volatile int keepBusy = 0;
		// Wait until stop request is cleared
		while (m_isRequestingStop) {
			if (keepBusy > 0) {
				std::cout << "keep busy, while waiting to stop";
			}
			m_isIdle = true;
		}
		// If idle, reload buffer data
		if (m_isIdle) {
			fileData = m_buffer->getData();
		}
		m_isIdle = false;
		float currSpeed = m_speed;
		int output_length = static_cast<int>(m_fftBufferSize * (1.0f / currSpeed));
		// Only process if enough space in ring buffer
		if (m_ringBuffer->getAvailableWrite() > output_length) {
			// Copy samples for FFT processing
			for (unsigned int i = 0; i < m_fftBufferSize; ++i) {
				if (i + m_currentPlayPosition < m_buffer->getNumFrames())
					FftData[i] = fileData[i + m_currentPlayPosition];
				else
					isFinished = true;
			}
			// Apply pitch correction if enabled
			if (m_usePitchCorrection)
				smb_PitchShift(m_pitch, m_fftBufferSize, m_fftSize, 16, m_fileSampleRate, FftData, FftData);
			// Interpolate output samples based on speed
			for (unsigned int i = 0; i < output_length; ++i) {
				float index = currSpeed * i;
				int hard_ind = static_cast<int>(index);
				float fac = index - hard_ind;
				if (hard_ind + 1 < m_fftBufferSize) {
					OutData[i] = (1.0f - fac) * FftData[hard_ind] + fac * FftData[hard_ind + 1];
				} else {
					OutData[i] = FftData[hard_ind];
				}
			}
			// Write processed samples to ring buffer
			if (m_ringBuffer->write(OutData, output_length))
				m_currentPlayPosition += m_fftBufferSize;
		}
		// If end of buffer reached, reset play position
		if (isFinished) {
			m_currentPlayPosition = 0;
			isFinished = false;
		}
	}
}

// Processes audio buffer by reading from ring buffer if not paused
void act::aio::TimeStretchingNode::process(ci::audio::Buffer* buffer) {
	if (!m_isPaused) {
		m_ringBuffer->read(buffer->getChannel(0), buffer->getNumFrames());
	}
	else {
		m_currentPlayPosition = 0;
	}
}
