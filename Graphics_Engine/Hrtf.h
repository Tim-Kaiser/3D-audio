#pragma once


#define SAF_USE_INTEL_MKL_LP64
#define SAF_ENABLE_SOFA_READER_MODULE

#include "saf.h"
#include "saf_externals.h"
#include <iostream>
#include <string>


class Hrtf {

public:

	Hrtf();
	~Hrtf();

	void LoadSOFA(char* file);

private:
	saf_sofa_container m_hSOFA;
	SAF_SOFA_READER_OPTIONS m_options;
};