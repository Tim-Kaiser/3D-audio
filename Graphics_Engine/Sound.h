#pragma once

#include <iostream>
#include <SDL.h>
#include <algorithm>
#include <fftw3.h>

class Sound {

static void callback(void* userdata, Uint8* stream, int len);

public:
	Sound();
	~Sound();

	void Play();
	void Pause();
	void setCoordinates(int elevation, int azimuth);
	void getFilename();


	int initAudio();
	void recalculateHRTF();
	void audioCallback(Uint8* outstream, int len);

private:
	int m_elevation;
	int m_azimuth;

	std::string m_filenameSample;
	std::string m_filenameHRIR;

	SDL_AudioDeviceID m_device;

	//Uint8* m_sample_freq_buffer;	
	
};