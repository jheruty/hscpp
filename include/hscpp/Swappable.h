#pragma once

#include <string>

namespace hscpp
{
	class ISwappable
	{
	public:
		virtual ~ISwappable() {};
		
		// Use 'Hscpp' prefix so as not to conflict with inheriting class' code.
		virtual std::string Hscpp_GetKey() = 0;
	};

	template <typename T>
	class Swappable : public ISwappable
	{
	public:
		static std::string s_Key;

		virtual std::string Hscpp_GetKey() override;
		// TODO: 
		// Register swappables in a list on construction, so they can be deleted by the hotswap system
		// later. Lists should be keyed off by the swappable's key.

	};

	template <typename T>
	std::string hscpp::Swappable<T>::Hscpp_GetKey()
	{
		return s_Key;
	}

	template <typename T>
	std::string hscpp::Swappable<T>::s_Key;

}