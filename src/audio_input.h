#ifndef AUDIO_INPUT_H
#define AUDIO_INPUT_H

#include "ofMain.h"
#include "RtAudio.h"
#include "chuck_fft.h"
#include "aubio.h"

/* Wrapper class around RtAudio. */
class AudioInput
{
public:
	/* Initializes RtAudio library. */
	AudioInput(long samplingRate, unsigned int numFrames);
	~AudioInput();

	/* Start listening for audio input. */
	void Start();

	/* Stops listening for audio input. */
	void Stop();
    
    /* Gets the maximum possible frequency that can be resolved by the
     * Fourier transform for the given sampling rate. */
    float GetFrequencyResolution();
    
    /* Gets the amplitude of the current mic input. */
    float GetCurrentAmplitude();
    
    /* Gets the pitch of the current mic input. Uses aubio library to
     * perform estimate. */
    float GetCurrentPitch();

	/* Gets a copy of current mic input. The returned buffer will contain 
     * at most |numFrames| valid entries, and must be freed by the user. */
	float* GetCurrentInput();
    
    /* Gets a Fourier-transformed version of mic input. The returned buffer
     * will contain |numFrames| complex numbers, which is |numFrames * 2|
     * floats. The buffer must be freed by the user. */
    complex* GetTransformedInput();

private:
	long samplingRate = 44100;
	unsigned int numFrames = 1024;

	/* Buffer to hold current audio input. */
	float* input;
    float* FFTWindow;
    
    /* Internal Aubio variables. */
    unsigned int windowSize;
    unsigned int hopSize;
    aubio_pitchdetection_mode mode;
    aubio_pitchdetection_type type;
    aubio_pitchdetection_t* pitchOutput;
    fvec_t* aubioInput = NULL;
    bool aubioInitFinished = false;

	/* Internal RtAudio object. */
	RtAudio audio;
};

#endif