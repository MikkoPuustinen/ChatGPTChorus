#define _USE_MATH_DEFINES

#include <iostream>

#include <cmath>
#include <array>
#include <math.h>
#include <string>
#include <fstream>
#include <vector>
#include "AudioFile.h"

// Define the maximum delay length and default delay value
constexpr int kMaxDelay = 1200;

const float min_delay = 0.01; // Minimum delay = 10 ms
const float max_delay = 0.025; // Maximum delay = 25 ms

// Define a circular buffer type for storing delay line samples
using DelayLine = std::array<float, kMaxDelay>;

// Initialize the circular buffer with the default delay value
DelayLine delay_line{ };

// Implement a function that applies a variable length delay to an input sample
float Delay(float input, float delay, int sampleRate) {
	// Add the input sample to the delay line
	delay_line[0] = input;

	// Calculate the position of the delayed sample in the delay line
	float delay_samples = delay * sampleRate;

	// Calculate the fractional part of the delay position using the fmod function
	float delay_frac = std::fmod(delay_samples, 1.0);

	// Read the samples surrounding the fractional delay position
	float sample1 = delay_line[std::floor(delay_samples)];
	float sample2 = delay_line[std::ceil(delay_samples)];

	// Use linear interpolation to estimate the value of the delayed sample
	float output = sample1 + delay_frac * (sample2 - sample1);

	// Shift all samples in the delay line by one position
	std::rotate(delay_line.begin(), delay_line.begin() + 1, delay_line.end());

	// Return the delayed sample
	return output;
}

// Implement a function that generates a modulated low-frequency oscillator (LFO) value
float ModulatedLfo(int time, float frequency, float depth) {
	// Compute the LFO value using the sine function
	float lfo = sin(2 * M_PI * frequency * time);

	// Modulate the LFO value by applying the sine function again
	float modulated = depth * sin(lfo);

	// Scale the modulated LFO value so that it varies between 0 and 1
	float scaled = (modulated + depth) / (2 * depth);

	return scaled;
}

int main() {
	// Define the path to the input WAV file
	std::string input_file_path = "./Audio Files/audio.wav";

	// Define the path to the output WAV file
	std::string output_file_path = "./Audio Files/output.wav";

	AudioFile<double> audioFile;
	audioFile.load(input_file_path);

	// Initialize the circular buffer with the default delay value
	DelayLine delay_line{};
	delay_line.fill(0.0f);

	int sampling_rate = audioFile.getSampleRate();
	// Compute the LFO frequency and delay time based on the sampling rate
	float lfo_frequency = 2.0 / sampling_rate;

	// Read the input samples from the WAV file
	for (int i = 0; i < audioFile.getNumSamplesPerChannel(); i++)
	{
		// Generate the modulated LFO value
		float lfo = ModulatedLfo(i, lfo_frequency, 0.1f);

		// Apply the modulated delay to the input sample
		float delay = min_delay + (max_delay - min_delay) * lfo;
		float delayed = Delay(audioFile.samples[0][i], delay, sampling_rate);

		// Mix the input sample and the delayed sample
		audioFile.samples[0][i] = audioFile.samples[0][i] + delayed;
	}

	audioFile.save(output_file_path);

	return 0;
}