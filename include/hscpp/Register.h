#pragma once

#include "Constructors.h"

#define HSCPP_REGISTER_HOTSWAPPABLE(T) \
namespace\
{\
	struct Hscpp_Registration\
	{\
		Hscpp_Registration()\
		{\
			hscpp::Constructors::RegisterConstructor<T>();\
		}\
	};\
}\
\
static Hscpp_Registration hscppAutoRegistration##T;