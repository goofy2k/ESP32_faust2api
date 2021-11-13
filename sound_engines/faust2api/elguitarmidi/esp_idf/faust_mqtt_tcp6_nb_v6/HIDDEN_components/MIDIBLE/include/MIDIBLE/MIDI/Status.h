#pragma once

#include "./Channel.h"
#include "./Type.h"

namespace m2d
{
namespace MIDIBLE
{
	namespace MIDI
	{
		class Status
		{
		public:
			enum Value : uint8_t
			{
				NoteOff = 0x80,
				NoteOn = 0x90,
				PolyphonicKeyPressure = 0xa0,
				ControlChange = 0xb0,
				ProgramChange = 0xc0,
				ChannelPressure = 0xd0,
				PitchWheelChange = 0xe0
			};

		private:
			Status::Value _rawValue;

		public:
			Status(Status::Value value)
			    : _rawValue(value)
			{
			}

			Status::Value rawValue() const
			{
				return _rawValue;
			}

			MIDI::Type toType() const
			{
				if ((this->rawValue() < 0x80) || (this->rawValue() == 0xf4) || (this->rawValue() == 0xf5) || (this->rawValue() == 0xf9) || (this->rawValue() == 0xfD)) {
					// Data bytes and undefined.
					return Type(Type::Value::Invalid);
				}
				if (this->rawValue() < 0xf0) {
					// Channel message, remove channel nibble.
					return Type(Type::Value(this->rawValue() & 0xf0));
				}

				return Type(Type::Value(this->rawValue()));
			}

			MIDI::Channel toChannel()
			{
				return Channel(Channel::Value((this->rawValue() & 0x0f) + 0x01));
			}

			static Status make(MIDI::Type type, MIDI::Channel channel)
			{
				uint8_t c = channel.rawValue();
				uint8_t t = type.rawValue();
				auto value = Status::Value((t & 0xf0) | ((c - 0x01) & 0x0f));
				return Status(value);
			}
		};
	}
}
}