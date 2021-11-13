#pragma once

namespace m2d
{
namespace MIDIBLE
{
	namespace MIDI
	{
		class Type
		{
		public:
			enum Value : uint8_t
			{
				Invalid = 0x00, ///< For notifying errors

				NoteOff = 0x80, ///< Note Off
				NoteOn = 0x90, ///< Note On
				AfterTouchPoly = 0xA0, ///< Polyphonic AfterTouch
				ControlChange = 0xB0, ///< Control Change / Channel Mode
				ProgramChange = 0xC0, ///< Program Change
				AfterTouchChannel = 0xD0, ///< Channel (monophonic) AfterTouch
				PitchBend = 0xE0, ///< Pitch Bend

				SystemExclusive = 0xF0, ///< System Exclusive
				SystemExclusiveStart = SystemExclusive,
				SystemExclusiveEnd = 0xF7, ///< System Exclusive End

				TimeCodeQuarterFrame = 0xF1, ///< System Common - MIDI Time Code Quarter Frame
				SongPosition = 0xF2, ///< System Common - Song Position Pointer
				SongSelect = 0xF3, ///< System Common - Song Select
				TuneRequest = 0xF6, ///< System Common - Tune Request

				Clock = 0xF8, ///< System Real Time - Timing Clock
				Tick = 0xF9, ///< System Real Time - Tick
				Start = 0xFA, ///< System Real Time - Start
				Continue = 0xFB, ///< System Real Time - Continue
				Stop = 0xFC, ///< System Real Time - Stop
				ActiveSensing = 0xFE, ///< System Real Time - Active Sensing
				SystemReset = 0xFF, ///< System Real Time - System Reset
			};

		private:
			Type::Value _rawValue;

		public:
			Type(Type::Value value)
			    : _rawValue(value)
			{
			}

			Type::Value rawValue() const
			{
				return _rawValue;
			}

			bool isChannelMessage()
			{
				return (this->rawValue() == Type::Value::NoteOff || this->rawValue() == Type::Value::NoteOn || this->rawValue() == Type::Value::ControlChange || this->rawValue() == Type::Value::AfterTouchPoly || this->rawValue() == Type::Value::AfterTouchChannel || this->rawValue() == Type::Value::PitchBend || this->rawValue() == Type::Value::ProgramChange);
			}
		};
	}
}
}