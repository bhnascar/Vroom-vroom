#include "audio_input.h"

/* Lock over input buffer. */
ofMutex mutex;

/* Callback function for RtAudio. Copies input to a global buffer. */
int callback(void* output, void* input, unsigned int numFrames,
             double streamTime, RtAudioStreamStatus status, void* data) 
{
	// Setup buffer pointers.
	// float corresponds to |RTAUDIO_FLOAT32|.
	float* externalBuffer = *(float**)data;
    float* outputBuffer = (float*)output;
    float* inputBuffer = (float*)input;
    
    mutex.lock();
    for (int i = 0; i < numFrames; i++) {
        outputBuffer[i] = 0.f;
        externalBuffer[i] = inputBuffer[i];
    }
    mutex.unlock();
    
    return 0;
}

AudioInput::AudioInput(long samplingRate, unsigned int numFrames)
	: samplingRate(samplingRate), numFrames(numFrames)
{
	// Check if the user has an input device.
	if (audio.getDeviceCount() < 1) {
        std::string warning = "You need at least one audio input device!";
    	std::cout << warning << std::endl;
    }

    // Enable log warnings.
    audio.showWarnings(true);

    // Allocate buffer for audio input frames
    input = new float[numFrames];
    memset(input, 0, numFrames * sizeof(float));
    
    // Initialize FFT window.
    FFTWindow = new float[numFrames];
    hanning(FFTWindow, numFrames);
    
    // Initialize Aubio variables.
    windowSize = numFrames;
    hopSize = windowSize / 4;
    mode = aubio_pitchm_freq;
    type = aubio_pitch_yinfft;
    aubioInput = new_fvec(hopSize, 1);
    pitchOutput = new_aubio_pitchdetection(windowSize, hopSize, 1, (ba_uint_t)samplingRate, type, mode);
    aubioInitFinished = true;
}

AudioInput::~AudioInput() 
{
	// Stop audio stream.
	Stop();

	// Close audio stream.
    if (audio.isStreamOpen()) {
        audio.closeStream();
    }

    mutex.lock();
    
    // Delete audio buffer
    delete[] input;
    
    // Delete FFT window
    delete[] FFTWindow;
    
    // Delete aubio variables.
    del_aubio_pitchdetection(pitchOutput);
    del_fvec(aubioInput);
    aubio_cleanup();
    aubioInitFinished = false;
    
    mutex.unlock();
}

void AudioInput::Start()
{
	// Abort if the audio stream is already running.
	if (audio.isStreamRunning()) {
		return;
	}

	// Setup input params.
    RtAudio::StreamParameters inputParams;
    inputParams.deviceId = audio.getDefaultInputDevice();
    inputParams.nChannels = 1;
    inputParams.firstChannel = 0;

    // Setup output params.
    RtAudio::StreamParameters outputParams;
    outputParams.deviceId = audio.getDefaultOutputDevice();
    outputParams.nChannels = 1;
    outputParams.firstChannel = 0;

    try {
        // Open audio stream.
        audio.openStream(&outputParams, &inputParams, 
                         RTAUDIO_FLOAT32, samplingRate,
                         &numFrames, &callback, (void*)&input);

        // Play audio stream.
        audio.startStream();
    }
    catch (int error) {
        std::cout << "Unexpected playback error";
        return;
    }
}

void AudioInput::Stop()
{
	if (audio.isStreamRunning()) {
		audio.stopStream();
	}
}

float AudioInput::GetFrequencyResolution() {
    return samplingRate / 2.f;
}

float AudioInput::GetCurrentAmplitude() {
    float rmsAmplitude = 0;
    mutex.lock();
    for (int i = 0; i < numFrames; i++) {
        rmsAmplitude += sqrt(input[i] * input[i]);
    }
    mutex.unlock();
    rmsAmplitude /= numFrames;
    return rmsAmplitude;
}

float AudioInput::GetCurrentPitch() {
    /* // Pitch extraction with aubio is more accurate, but
       // things crash every now and then. Haven't been able
       // to track down what's causing the memory smashing.
    mutex.lock();
    if (!aubioInitFinished) {
        mutex.unlock();
        return 0.f;
    }
    float rmsAmplitude = 0;
    for (int i = 0; i < numFrames; i++) {
        rmsAmplitude += sqrt(input[i] * input[i]);
        aubioInput->data[0][i] = input[i];
    }
    rmsAmplitude /= numFrames;
    float result = (rmsAmplitude > 0.01) ? aubio_pitchdetection(pitchOutput, aubioInput) : 0.f;
    mutex.unlock();
    return result; */
    
    // Return the frequency of the bin with the strongest signal.
    // Frequency range of bins is 0 to (samplingRate / 2.f).
    complex* input = GetTransformedInput();
    float maxValue = 0;
    float maxIndex = 0;
    float rmsAmplitude = 0;
    for (int i = 0; i < numFrames; i++) {
        complex value = input[i];
        float abs = cmp_abs(value);
        if (abs > maxValue) {
            maxValue = abs;
            maxIndex = i;
        }
    }
    float amplitude = GetCurrentAmplitude();
    float freq = (maxIndex / numFrames) * GetFrequencyResolution();
    return (amplitude > 0.01) ? freq : 0.f;
}

float* AudioInput::GetCurrentInput()
{
    float* inputCopy = new float[numFrames];
    mutex.lock();
    memcpy(inputCopy, input, sizeof(float) * numFrames);
    mutex.unlock();
	return inputCopy;
}

complex* AudioInput::GetTransformedInput()
{
    // Copy input to float buffer of twice the length.
    float* inputCopy = new float[numFrames * 2];
    memset(inputCopy, 0, sizeof(float) * numFrames * 2);
    mutex.lock();
    memcpy(inputCopy, input, sizeof(float) * numFrames);
    mutex.unlock();
    
    // Apply FFT window and perform FFT.
    apply_window(inputCopy, FFTWindow, numFrames);
    rfft(inputCopy, numFrames, FFT_FORWARD);
    return (complex*)inputCopy;
}