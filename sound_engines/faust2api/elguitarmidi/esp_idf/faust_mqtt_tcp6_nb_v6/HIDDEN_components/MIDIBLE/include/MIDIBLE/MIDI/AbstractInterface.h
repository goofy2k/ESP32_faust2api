#pragma once

#include <functional>

#include "./Channel.h"
#include "./PitchBend.h"
#include "./Status.h"
#include "./Type.h"

namespace m2d
{
namespace MIDIBLE
{
	namespace MIDI
	{
		class AbstractInterface
		{
		protected:
			int runningStatus;
			bool thruActivated;

		public:
	
			AbstractInterface()
			{
			}

			// sending
			void sendNoteOn(uint8_t note, uint8_t velocity, MIDI::Channel channel)
			{
				this->sendChannelMessage(Type::Value::NoteOn, note, velocity, channel);
			}

			void sendNoteOff(uint8_t note, uint8_t velocity, Channel channel)
			{
				this->sendChannelMessage(Type::Value::NoteOff, note, velocity, channel);
			}

			void sendProgramChange(uint8_t number, Channel channel)
			{
				this->sendChannelMessage(Type::Value::ProgramChange, number, 0, channel);
			}

			void sendControlChange(uint8_t number, uint8_t value, Channel channel)
			{
				this->sendChannelMessage(Type::Value::ControlChange, number, value, channel);
			}

			void sendPitchBend(int value, Channel channel)
			{
				const unsigned bend = value - MIDI::PitchBend::Min;
				this->sendChannelMessage(Type::Value::PitchBend, (bend & 0x7f), (bend >> 7) & 0x7f, channel);
			}

			void sendPitchBend(double pitchValue, Channel channel)
			{
				const int scale = pitchValue > 0.0 ? MIDI::PitchBend::Max : MIDI::PitchBend::Min;
				const int value = int(pitchValue * double(scale));
				this->sendPitchBend(value, channel);
			}

			void sendPolyPressure(uint8_t note, uint8_t pressure, Channel channel)
			{
				this->sendChannelMessage(Type::Value::AfterTouchPoly, note, pressure, channel);
			}

			void sendAfterTouch(uint8_t pressure, Channel channel)
			{
				this->sendChannelMessage(Type::Value::AfterTouchChannel, pressure, 0, channel);
			}

			void sendAfterTouch(uint8_t note, uint8_t pressure, Channel channel)
			{
				this->sendChannelMessage(Type::Value::AfterTouchChannel, note, pressure, channel);
			}

			void sendSysEx(const uint8_t *, uint16_t inLength)
			{
				// TODO
			}

			void sendTimeCodeQuarterFrame(uint8_t typeNibble, uint8_t valuesNibble)
			{
				const uint8_t data = uint8_t((((typeNibble & 0x07) << 4) | (valuesNibble & 0x0f)));
				this->sendTimeCodeQuarterFrame(data);
			}

			void sendTimeCodeQuarterFrame(uint8_t data)
			{
				this->sendSystemCommonMessage(Type::Value::TimeCodeQuarterFrame, data);
			}

			void sendSongPosition(unsigned short beats)
			{
				uint8_t data1 = beats & 0x7f;
				uint8_t data2 = (beats >> 7) & 0x7f;

				this->sendSystemCommonMessage(Type::Value::SongPosition, data1, data2);
			}

			void sendSongSelect(uint8_t number)
			{
				this->sendSystemCommonMessage(Type::Value::SongSelect, number & 0x7f);
			}

			void sendTuneRequest()
			{
				this->sendSystemCommonMessage(Type::Value::TuneRequest);
			}

			void sendActiveSensing()
			{
				this->sendSystemCommonMessage(Type::Value::ActiveSensing);
			}

			void sendStart()
			{
				this->sendRealTimeMessage(Type::Value::Start);
			}

			void sendContinue()
			{
				this->sendRealTimeMessage(Type::Value::Continue);
			}

			void sendStop()
			{
				this->sendRealTimeMessage(Type::Value::Stop);
			}

			void sendClock()
			{
				this->sendRealTimeMessage(Type::Value::Clock);
			}

			void sendTick()
			{
				this->sendRealTimeMessage(Type::Value::Tick);
			}

			void sendReset()
			{
				this->sendRealTimeMessage(Type::Value::SystemReset);
			}

		protected:
			// Channel messages
			virtual void sendChannelMessage(Type type, uint8_t data1, uint8_t data2, Channel channel)
			{
				// Then test if channel is valid
				if (channel.rawValue() >= Channel::Value::Off || channel.rawValue() == Channel::Value::Omni || type.rawValue() < 0x80) {
					return; // Don't send anything
				}

				if (type.rawValue() <= Type::Value::PitchBend) {
					// Channel messages

					// Protection: remove MSBs on data
					data1 &= 0x7f;
					data2 &= 0x7f;

					const Status status = Status::make(type, channel);

					if (type.rawValue() != Type::Value::ProgramChange && type.rawValue() != Type::Value::AfterTouchChannel) {
						this->serialize(status.rawValue(), data1, data2);
					}
					else {
						this->serialize(status.rawValue(), data1);
					}
				}
				else if (type.rawValue() >= Type::Value::Clock && type.rawValue() <= Type::Value::SystemReset) {
					this->sendRealTimeMessage(type.rawValue()); // System Real-time and 1 byte.
				}
			}

			// SystemCommon message
			virtual void sendSystemCommonMessage(Type type, uint8_t data1 = 0, uint8_t data2 = 0)
			{
			}

			// RealTime messages
			virtual void sendRealTimeMessage(Type type)
			{
				// Do not invalidate Running Status for real-time messages
				// as they can be interleaved within any message.

				switch (type.rawValue()) {
					case Type::Value::Clock:
					case Type::Value::Start:
					case Type::Value::Stop:
					case Type::Value::Continue:
					case Type::Value::ActiveSensing:
					case Type::Value::SystemReset:
						this->serialize(type.rawValue());
						break;
					default:
						// Invalid Real Time marker
						break;
				}
			}

			// serialize towards to hardware
			virtual void serialize(uint8_t) = 0;
			virtual void serialize(uint8_t, uint8_t) = 0;
			virtual void serialize(uint8_t, uint8_t, uint8_t) = 0;
		};
	}
}
}