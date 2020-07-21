#pragma once

namespace hscpp
{
	class ISwappable
	{
	public:
		virtual ~ISwappable() {};
	};

	template <typename T>
	class Swappable : public ISwappable
	{

	};
}