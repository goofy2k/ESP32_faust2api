#pragma once

namespace m2d
{
namespace MIDIBLE
{
	namespace MIDI
	{
		class Channel
		{
		public:
			enum Value : uint8_t
			{
				// MIDI Channel enumeration values
				Omni = 0x0,
				C1 = 0x0,
				C2 = 0x1,
				C3 = 0x2,
				C4 = 0x3,
				C5 = 0x4,
				C6 = 0x5,
				C7 = 0x6,
				C8 = 0x7,
				C9 = 0x8,
				C10 = 0x9,
				C11 = 0xa,
				C12 = 0xb,
				C13 = 0xc,
				C14 = 0xd,
				C15 = 0xe,
				C16 = 0xf,
				Base = 0x10,
				All = 0x1f,
				Off = 0x1f
			};

		private:
			Channel::Value _rawValue;

		public:
			Channel(Channel::Value value)
			    : _rawValue(value)
			{
			}

			Channel::Value rawValue() const
			{
				return _rawValue;
			}
		};
	}
}
}