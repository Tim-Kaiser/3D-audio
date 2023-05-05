#pragma once


#define SAF_USE_INTEL_MKL_LP64
#define SAF_ENABLE_SIMD 
#define SAF_USE_FFTW
#define SAF_ENABLE_SOFA_READER_MODULE

#include "saf.h"
#include <string>


class Hrtf {

public:

	Hrtf();
	~Hrtf();

	void LoadSOFA(std::string* file);

private:

};