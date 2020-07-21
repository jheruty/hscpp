#pragma once

#include "hscpp/Constructors.h"

namespace hscpp
{

	hscpp::ISwappable* Constructors::Create(const std::type_index& type)
	{
		auto constructorIt = GetConstructorsByType().find(type);
		if (constructorIt != GetConstructorsByType().end()) {
			IConstructor* constructor = GetConstructors().at(constructorIt->second).get();
			return constructor->Construct();
		}

		return nullptr;
	}

	std::vector<std::unique_ptr<hscpp::IConstructor>>& Constructors::GetConstructors()
	{
		static std::vector<std::unique_ptr<hscpp::IConstructor>> constructors;
		return constructors;
	}

	std::unordered_map<std::type_index, size_t>& Constructors::GetConstructorsByType()
	{
		static std::unordered_map<std::type_index, size_t> iConstructorByType;
		return iConstructorByType;
	}

}