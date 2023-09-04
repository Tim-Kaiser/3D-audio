#include "Sound.h"
#include <vector>


const int SAMPLERATE = 44100;
const int SAMPLES_PER_FILL = 128;
const int SAMPLESIZE = sizeof(float);

fftw_complex resultHRTF[SAMPLES_PER_FILL];

int totalSamples;

std::vector<float> spatialized_sample_right;
std::vector<float> spatialized_sample_left;


void Sound::callback(void* userdata, Uint8* stream, int len)
{
	const auto sound = reinterpret_cast<Sound*>(userdata);
	sound->audioCallback(stream, len);

}

Sound::Sound()
{
	m_elevation = 10;
	m_azimuth = 0;
	m_filenameSample = "./Audio/snare-dist03.wav";
	//m_filenameSample = "./Audio/Heavy.wav";

	m_filenameHRIR = "./mit/elev10/H10e000a.wav";

	SDL_Init(SDL_INIT_AUDIO);
}

Sound::~Sound()
{

}

void Sound::Play()
{
	SDL_PauseAudioDevice(m_device, 0);
}

void Sound::Pause()
{
	SDL_PauseAudioDevice(m_device, 1);

}

void Sound::setCoordinates(int elevation, int azimuth)
{
	m_elevation = elevation;
	m_azimuth = azimuth;
	getFilename();
	recalculateHRTF();
}

void Sound::getFilename()
{
}

int Sound::initAudio()
{
	// SDL SETUP
	SDL_AudioSpec obtained_audio_spec;
	SDL_AudioSpec desired_audio_spec;

	desired_audio_spec.freq = SAMPLERATE;
	desired_audio_spec.format = AUDIO_F32LSB;
	desired_audio_spec.channels = 2;
	desired_audio_spec.samples = SAMPLES_PER_FILL;
	desired_audio_spec.callback = callback;
	desired_audio_spec.userdata = this;

	const char* name = SDL_GetAudioDeviceName(3, 0);
	m_device = SDL_OpenAudioDevice(name, 0, &desired_audio_spec, &obtained_audio_spec, 0);

	std::cout << "Opened AudioDevice: " << name << std::endl;

	//=====================================================================================================

	// HRIR
	SDL_AudioCVT hrtf_audio_cvt;
	SDL_AudioSpec hrtf_audio_spec;

	Uint8* hrir_source_buffer;
	Uint32 hrir_source_len;

	// SAMPLE
	SDL_AudioCVT sample_audio_cvt;
	SDL_AudioSpec sample_audio_spec;

	Uint8* sample_source_buffer;
	Uint32 sample_source_len;


	// LOAD WAVS

	bool loadedSample = SDL_LoadWAV(m_filenameSample.c_str(), &sample_audio_spec, &sample_source_buffer, &sample_source_len);

	if (!loadedSample) {
		std::cout << "Could not load audio sample file!" << std::endl;
		return 0;
	}

	bool loadedHRIR = SDL_LoadWAV(m_filenameHRIR.c_str(), &hrtf_audio_spec, &hrir_source_buffer, &hrir_source_len);

	if (!loadedHRIR) {
		std::cout << "Could not load HRIR file!" << std::endl;
		return 0;
	}

	int padded_size = SAMPLES_PER_FILL * 2 - 1;

	// DFT HRIR

	fftw_complex sourceHRTF[SAMPLES_PER_FILL];


	fftw_plan planHRIR = fftw_plan_dft_1d(SAMPLES_PER_FILL, sourceHRTF, resultHRTF, FFTW_FORWARD, FFTW_ESTIMATE);


	SDL_BuildAudioCVT(&hrtf_audio_cvt, hrtf_audio_spec.format, hrtf_audio_spec.channels, hrtf_audio_spec.freq, AUDIO_F32LSB, 2, hrtf_audio_spec.freq);
	Uint32 source_len_mult = hrtf_audio_cvt.len_mult;
	hrtf_audio_cvt.buf = static_cast<Uint8*>(malloc(hrir_source_len * source_len_mult));

	hrtf_audio_cvt.len = hrir_source_len;
	memcpy(hrtf_audio_cvt.buf, hrir_source_buffer, hrir_source_len);
	SDL_ConvertAudio(&hrtf_audio_cvt);

	memcpy(sourceHRTF, hrtf_audio_cvt.buf, hrir_source_len);


	fftw_execute(planHRIR);



	//// DFT SAMPLE


	fftw_complex sourceSample[SAMPLES_PER_FILL];
	fftw_complex resultSample[SAMPLES_PER_FILL];


	// sample is loaded as mono and turned into stereo with hrtf
	SDL_BuildAudioCVT(&sample_audio_cvt, sample_audio_spec.format, sample_audio_spec.channels, sample_audio_spec.freq, AUDIO_F32LSB, 1, sample_audio_spec.freq);
	source_len_mult = sample_audio_cvt.len_mult;
	sample_audio_cvt.buf = static_cast<Uint8*>(malloc(sample_source_len * source_len_mult));
	sample_audio_cvt.len = sample_source_len;

	memcpy(sample_audio_cvt.buf, sample_source_buffer, sample_source_len);
	SDL_ConvertAudio(&sample_audio_cvt);
	

	int chunks = sample_source_len / SAMPLES_PER_FILL;

	// set size, performance
	spatialized_sample_right.resize(sample_source_len);
	spatialized_sample_left.resize(sample_source_len);

	// DELETE THIS
	std::vector<float> v( (float*)sample_audio_cvt.buf, (float*)(sample_audio_cvt.buf + sample_source_len));
	//spatialized_sample_right = v;
	//spatialized_sample_left = v;

	totalSamples = sample_source_len / SAMPLESIZE;

	for (int i = 0; i < chunks; i++) {
		int dist = SAMPLESIZE * SAMPLES_PER_FILL;
		Uint8* audio_buf_chunk = static_cast<Uint8*>(malloc(dist));
		Uint8* start = sample_audio_cvt.buf + (i * dist);
		Uint8* end = start + dist;

		std::copy(start, end, audio_buf_chunk);
		memcpy(sourceSample, audio_buf_chunk, dist);

		fftw_plan planSample = fftw_plan_dft_1d(SAMPLES_PER_FILL, sourceSample, resultSample, FFTW_FORWARD, FFTW_ESTIMATE);
		fftw_execute(planSample);

		// FFTW PLAN SETUP
		fftw_complex freq_left[SAMPLES_PER_FILL];
		fftw_complex time_left[SAMPLES_PER_FILL];

		fftw_complex freq_right[SAMPLES_PER_FILL];
		fftw_complex time_right[SAMPLES_PER_FILL];

		fftw_plan plan_inverse_left = fftw_plan_dft_1d(SAMPLES_PER_FILL, freq_left, time_left, FFTW_BACKWARD, FFTW_ESTIMATE);
		fftw_plan plan_inverse_right = fftw_plan_dft_1d(SAMPLES_PER_FILL, freq_right, time_right, FFTW_BACKWARD, FFTW_ESTIMATE);

		for (int j = 0; j < SAMPLES_PER_FILL; j++) {

			//// real
			//freq_left[i][0] = (resultSample[i][0] * resultHRTF[i][0]) - (resultSample[i][1] * resultHRTF[i][1]);
			//// imaginary
			//freq_left[i][1] = (resultSample[i][0] * resultHRTF[i][1]) + (resultSample[i][1] * resultHRTF[i][0]);

			//// real
			//freq_right[i][0] = (resultSample[i][0] * resultHRTF[i][0]) - (resultSample[i][1] * resultHRTF[i][1]);
			//// imaginary
			//freq_right[i][1] = (resultSample[i][0] * resultHRTF[i][1]) + (resultSample[i][1] * resultHRTF[i][0]);

			freq_left[j][0] = resultSample[j][0];
			freq_left[j][1] = resultSample[j][1];

			freq_right[j][0] = resultSample[j][0];
			freq_right[j][1] = resultSample[j][1];
		}

		fftw_execute(plan_inverse_left);
		fftw_execute(plan_inverse_right);

		for (int k = 0; k < SAMPLES_PER_FILL; k++) {
			// normalize data
			//std::cout << (float)time_right[k][0] / SAMPLES_PER_FILL << std::endl;
			spatialized_sample_right[SAMPLES_PER_FILL * i + k] = (float)(time_right[k][0]) / (SAMPLES_PER_FILL);
			spatialized_sample_left[SAMPLES_PER_FILL * i + k] = (float)(time_left[k][0]) / (SAMPLES_PER_FILL);
		}
	}

	return 1;

}

void Sound::recalculateHRTF()
{
}

// len: how many bytes to write to stream
void Sound::audioCallback(Uint8* outstream, int len)
{
	static int currentSample;
	// loop
	if (currentSample >= totalSamples) {
		currentSample = 0;
	}

	// number of bytes / size of sample / number of channels
	int samples_to_write = len / SAMPLESIZE / 2;

	if (currentSample + samples_to_write > totalSamples) {
		samples_to_write = totalSamples - currentSample;
	}



	for (int i = 0; i < samples_to_write; i++) {
		//std::cout << spatialized_sample_right[currentSample + i] << std::endl;
		((float*)outstream)[i * 2] = spatialized_sample_right[currentSample + i];
		// CHANGE THIS TO LEFT
		((float*)outstream)[i * 2 + 1] = spatialized_sample_left[currentSample + i];
	}

	currentSample += samples_to_write;
}

