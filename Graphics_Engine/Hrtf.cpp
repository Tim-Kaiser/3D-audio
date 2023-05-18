#include "Hrtf.h"


Hrtf::Hrtf()
{
	m_options = SAF_SOFA_READER_OPTION_NETCDF;
}

Hrtf::~Hrtf()
{
}

void Hrtf::LoadSOFA(char* file)
{
	saf_sofa_container sofa;

	//SAF_SOFA_ERROR_CODES error = saf_sofa_open(&sofa, file, m_options);
	//std::cout << error << std::endl;
	
}
