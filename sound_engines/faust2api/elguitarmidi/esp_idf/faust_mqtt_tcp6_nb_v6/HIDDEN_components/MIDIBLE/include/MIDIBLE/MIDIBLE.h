#pragma once

//#include <BLEAdvertising.h>  //FCKX
//#include <BLEDevice.h>       //FCKX
#include <functional>
#include <string>

#include <esp_timer.h>

#include "./MIDI/AbstractInterface.h"

namespace m2d
{
namespace MIDIBLE
{
	namespace Const
	{
		const static std::string ServiceUUID = "03b80e5a-ede8-4b33-a751-6ce34ec4c700";
		const static std::string CharacteristicUUID = "7772e5db-3868-4112-a1a9-f2669d106bf3";
	}

	class ServerCallbacks //: public BLEServerCallbacks  //FCKX
	{
	public:
		std::function<void()> connect_handler = nullptr;
		std::function<void()> disconnect_handler = nullptr;

		void onConnect(BLEServer *server)
		{
			if (connect_handler) {
				connect_handler();
			}
		}

		void onDisconnect(BLEServer *server)
		{
			if (disconnect_handler) {
				disconnect_handler();
			}
		}
	};

	class CharacteristicCallbacks : public BLECharacteristicCallbacks
	{
	public:
		std::function<void(std::string)> read_handler = [](std::string) {};
		std::function<void(std::string)> write_handler = [](std::string) {};

		void onRead(BLECharacteristic *characteristic)
		{
			read_handler(characteristic->getValue());
		}

		void onWrite(BLECharacteristic *characteristic)
		{
			write_handler(characteristic->getValue());
		}
	};

	class BLEInterface : public MIDI::AbstractInterface
	{
	public:
		std::function<void(MIDI::Channel channel, uint8_t note, uint8_t velocity)> note_on_handler = nullptr;
		std::function<void(MIDI::Channel channel, uint8_t note, uint8_t velocity)> note_off_handler = nullptr;
		std::function<void(MIDI::Channel channel, uint8_t note, uint8_t velocity)> after_touch_poly_handler = nullptr;
		std::function<void(MIDI::Channel, uint8_t, uint8_t)> control_change_handler = nullptr;
		std::function<void(MIDI::Channel, uint8_t)> program_change_handler = nullptr;
		std::function<void(MIDI::Channel, uint8_t)> after_touch_channel_handler = nullptr;
		std::function<void(MIDI::Channel, uint8_t)> pitch_bend_handler = nullptr;
		std::function<void(unsigned short beats)> song_position_handler = nullptr;
		std::function<void(unsigned short song_number)> song_select_handler = nullptr;
		std::function<void(void)> tune_request_handler = nullptr;
		std::function<void(uint8_t)> time_code_quarter_frame_handler = nullptr;
		std::function<void(const uint8_t *array, uint16_t size)> sys_ex_handler = nullptr;
		std::function<void(void)> clock_handler = nullptr;
		std::function<void(void)> start_handler = nullptr;
		std::function<void(void)> continue_handler = nullptr;
		std::function<void(void)> stop_handler = nullptr;
		std::function<void(void)> active_sensing_handler = nullptr;
		std::function<void(void)> reset_handler = nullptr;

	protected:
		uint8_t midiPacket[5];
		BLEServer *server;
		BLEService *midi_service;
		BLECharacteristic *midi_characteristic;
		std::string name;
		uint8_t device_udid;

		ServerCallbacks service_callbacks;
		CharacteristicCallbacks characteristic_callbacks;

		inline static void getMidiTimestamp(uint8_t *header, uint8_t *timestamp)
		{
			/*
         The first byte of all BLE packets must be a header byte. This is followed by timestamp bytes and MIDI messages.
         Header Byte
         bit 7     Set to 1.
         bit 6     Set to 0. (Reserved for future use)
         bits 5-0  timestampHigh:Most significant 6 bits of timestamp information.
         The header byte contains the topmost 6 bits of timing information for MIDI events in the BLE
         packet. The remaining 7 bits of timing information for individual MIDI messages encoded in a
         packet is expressed by timestamp bytes.
         Timestamp Byte
         bit 7       Set to 1.
         bits 6-0    timestampLow: Least Significant 7 bits of timestamp information.
         The 13-bit timestamp for the first MIDI message in a packet is calculated using 6 bits from the
         header byte and 7 bits from the timestamp byte.
         Timestamps are 13-bit values in milliseconds, and therefore the maximum value is 8,191 ms.
         Timestamps must be issued by the sender in a monotonically increasing fashion.
         timestampHigh is initially set using the lower 6 bits from the header byte while the timestampLow is
         formed of the lower 7 bits from the timestamp byte. Should the timestamp value of a subsequent
         MIDI message in the same packet overflow/wrap (i.e., the timestampLow is smaller than a
         preceding timestampLow), the receiver is responsible for tracking this by incrementing the
         timestampHigh by one (the incremented value is not transmitted, only understood as a result of the
         overflow condition).
         In practice, the time difference between MIDI messages in the same BLE packet should not span
         more than twice the connection interval. As a result, a maximum of one overflow/wrap may occur
         per BLE packet.
         Timestamps are in the sender’s clock domain and are not allowed to be scheduled in the future.
         Correlation between the receiver’s clock and the received timestamps must be performed to
         ensure accurate rendering of MIDI messages, and is not addressed in this document.
         */
			/*
         Calculating a Timestamp
         To calculate the timestamp, the built-in millis() is used.
         The BLE standard only specifies 13 bits worth of millisecond data though,
         so it’s bitwise anded with 0x1FFF for an ever repeating cycle of 13 bits.
         This is done right after a MIDI message is detected. It’s split into a 6 upper bits, 7 lower bits,
         and the MSB of both bytes are set to indicate that this is a header byte.
         Both bytes are placed into the first two position of an array in preparation for a MIDI message.
         */

			auto currentTimeStamp = esp_timer_get_time() & 0x01FFF;
			*header = ((currentTimeStamp >> 7) & 0x3F) | 0x80; // 6 bits plus MSB
			*timestamp = (currentTimeStamp & 0x7F) | 0x80; // 7 bits plus MSB
		}

		// serialize towards hardware

		void serialize(uint8_t b1)
		{
			getMidiTimestamp(&this->midiPacket[0], &this->midiPacket[1]);

			this->midiPacket[2] = b1;

			// TODO: quid running status

			this->midi_characteristic->setValue(this->midiPacket, 3);
			this->midi_characteristic->notify();
		}

		void serialize(uint8_t b1, uint8_t b2)
		{
			getMidiTimestamp(&this->midiPacket[0], &this->midiPacket[1]);

			this->midiPacket[2] = b1;
			this->midiPacket[3] = b2;

			// TODO: quid running status

			this->midi_characteristic->setValue(this->midiPacket, 4);
			this->midi_characteristic->notify();
		}

		void serialize(uint8_t b1, uint8_t b2, uint8_t b3)
		{
			getMidiTimestamp(&this->midiPacket[0], &this->midiPacket[1]);

			this->midiPacket[2] = b1;
			this->midiPacket[3] = b2;
			this->midiPacket[4] = b3;

			// TODO: quid running status

			this->midi_characteristic->setValue(this->midiPacket, 5);
			this->midi_characteristic->notify();
		}

	public:
		BLEInterface(std::string name, uint16_t udid)
		{
			this->name = name;
			this->device_udid = udid;

			BLEDevice::init(this->name);
			this->server = BLEDevice::createServer();
			this->server->setCallbacks(&service_callbacks);
			this->midi_service = server->createService(Const::ServiceUUID.c_str());
			this->midi_characteristic = midi_service->createCharacteristic(Const::CharacteristicUUID.c_str(),
			    BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY | BLECharacteristic::PROPERTY_WRITE_NR);

			this->characteristic_callbacks.write_handler = [&](std::string rxValue) {
				if (rxValue.length() > 0) {
					this->receive((uint8_t *)(rxValue.c_str()), rxValue.length());
				}
			};
			this->midi_characteristic->setCallbacks(&characteristic_callbacks);
		}

		~BLEInterface()
		{
		}

		void begin()
		{
			this->midi_service->start();

			BLEAdvertising *advertising = this->server->getAdvertising();
			BLEAdvertisementData advertising_data;

			auto udid = std::string((const char *)&this->device_udid);
			advertising_data.setCompleteServices(BLEUUID(Const::ServiceUUID));
			advertising->setAdvertisementData(advertising_data);
			this->server->startAdvertising();
		}

		void parse(MIDI::Status status, uint8_t data1 = 0, uint8_t data2 = 0)
		{
			MIDI::Type type = status.toType();
			MIDI::Channel channel = status.toChannel();

			switch (type.rawValue()) {
				case MIDI::Type::Value::NoteOff:
					if (this->note_off_handler) {
						this->note_off_handler(channel, data1, data2);
					}
					break;
				case MIDI::Type::Value::NoteOn:
					if (this->note_on_handler) {
						this->note_on_handler(channel, data1, data2);
					}
					break;
				case MIDI::Type::Value::AfterTouchPoly:
					if (this->after_touch_poly_handler) {
						this->after_touch_poly_handler(channel, data1, data2);
					}
					break;
				case MIDI::Type::Value::ControlChange:
					if (this->control_change_handler) {
						this->control_change_handler(channel, data1, data2);
					}
					break;
				case MIDI::Type::Value::ProgramChange:
					if (this->program_change_handler) {
						this->program_change_handler(channel, data1);
					}
					break;
				case MIDI::Type::Value::AfterTouchChannel:
					if (this->after_touch_channel_handler) {
						this->after_touch_channel_handler(channel, data1);
					}
					break;
				case MIDI::Type::Value::PitchBend:
					if (this->pitch_bend_handler) {
						int value = (int)((data1 & 0x7f) | ((data2 & 0x7f) << 7)) + MIDI::PitchBend::Min;
						this->pitch_bend_handler(channel, value);
					}
					break;
				case MIDI::Type::Value::SystemExclusive:
					break;
				case MIDI::Type::Value::TimeCodeQuarterFrame:
					if (this->time_code_quarter_frame_handler) {
						this->time_code_quarter_frame_handler(data1);
					}
					break;
				case MIDI::Type::Value::SongPosition:
					if (this->song_position_handler) {
						unsigned short value = unsigned((data1 & 0x7f) | ((data2 & 0x7f) << 7));
						this->song_position_handler(value);
					}
					break;
				case MIDI::Type::Value::SongSelect:
					if (this->song_select_handler) {
						this->song_select_handler(data1);
					}
					break;
				case MIDI::Type::Value::TuneRequest:
					if (this->tune_request_handler) {
						this->tune_request_handler();
					}
					break;
				case MIDI::Type::Value::Clock:
					if (this->clock_handler) {
						this->clock_handler();
					}
					break;
				case MIDI::Type::Value::Tick:
					break;
				case MIDI::Type::Value::Start:
					if (this->start_handler) {
						this->start_handler();
					}
					break;
				case MIDI::Type::Value::Continue:
					if (this->continue_handler) {
						this->continue_handler();
					}
					break;
				case MIDI::Type::Value::Stop:
					if (this->stop_handler) {
						this->stop_handler();
					}
					break;
				case MIDI::Type::Value::ActiveSensing:
					if (this->active_sensing_handler) {
						this->active_sensing_handler();
					}
					break;
				case MIDI::Type::Value::SystemReset:
					if (this->reset_handler) {
						this->reset_handler();
					}
					break;
				case MIDI::Type::Value::SystemExclusiveEnd:
				case MIDI::Type::Value::Invalid:
					break;
			}
		}

		void receive(uint8_t *buffer, uint8_t bufferSize)
		{
			/*
			The general form of a MIDI message follows:
			n-byte MIDI Message
			Byte 0            MIDI message Status byte, Bit 7 is Set to 1.
			Bytes 1 to n-1    MIDI message Data bytes, if n > 1. Bit 7 is Set to 0
			There are two types of MIDI messages that can appear in a single packet: full MIDI messages and
			Running Status MIDI messages. Each is encoded differently.
			A full MIDI message is simply the MIDI message with the Status byte included.
			A Running Status MIDI message is a MIDI message with the Status byte omitted. Running Status
			MIDI messages may only be placed in the data stream if the following criteria are met:
			1.  The original MIDI message is 2 bytes or greater and is not a System Common or System
			Real-Time message.
			2.  The omitted Status byte matches the most recently preceding full MIDI message’s Status
			byte within the same BLE packet.
			In addition, the following rules apply with respect to Running Status:
			1.  A Running Status MIDI message is allowed within the packet after at least one full MIDI
			message.
			2.  Every MIDI Status byte must be preceded by a timestamp byte. Running Status MIDI
			messages may be preceded by a timestamp byte. If a Running Status MIDI message is not
			preceded by a timestamp byte, the timestamp byte of the most recently preceding message
			in the same packet is used.
			3.  System Common and System Real-Time messages do not cancel Running Status if
			interspersed between Running Status MIDI messages. However, a timestamp byte must
			precede the Running Status MIDI message that follows.
			4.  The end of a BLE packet does cancel Running Status.
			In the MIDI 1.0 protocol, System Real-Time messages can be sent at any time and may be
			inserted anywhere in a MIDI data stream, including between Status and Data bytes of any other
			MIDI messages. In the MIDI BLE protocol, the System Real-Time messages must be deinterleaved
			from other messages – except for System Exclusive messages.
			*/

			//Pointers used to search through payload.
			uint8_t lPtr = 0;
			uint8_t rPtr = 0;

			//lastStatus used to capture runningStatus
			uint8_t lastStatus;

			//Decode first packet -- SHALL be "Full MIDI message"
			lPtr = 2; //Start at first MIDI status -- SHALL be "MIDI status"

			//While statement contains incrementing pointers and breaks when buffer size exceeded.
			while (1) {
				lastStatus = buffer[lPtr];
				if ((buffer[lPtr] < 0x80)) {
					//Status message not present, bail
					return;
				}
				//Point to next non-data byte
				rPtr = lPtr;
				while ((buffer[rPtr + 1] < 0x80) && (rPtr < (bufferSize - 1))) {
					rPtr++;
				}
				//look at l and r pointers and decode by size.
				if (rPtr - lPtr < 1) {
					//Time code or system
					this->parse(MIDI::Status::Value(lastStatus));
				}
				else if (rPtr - lPtr < 2) {
					this->parse(MIDI::Status::Value(lastStatus), buffer[lPtr + 1]);
				}
				else if (rPtr - lPtr < 3) {
					this->parse(MIDI::Status::Value(lastStatus), buffer[lPtr + 1], buffer[lPtr + 2]);
				}
				else {
					//Too much data
					//If not System Common or System Real-Time, send it as running status
					switch (MIDI::Type(MIDI::Type::Value(buffer[lPtr] & 0xF0)).rawValue()) {
						case MIDI::Type::Value::NoteOff:
						case MIDI::Type::Value::NoteOn:
						case MIDI::Type::Value::AfterTouchPoly:
						case MIDI::Type::Value::ControlChange:
						case MIDI::Type::Value::PitchBend:
							for (int i = lPtr; i < rPtr; i = i + 2)
								this->parse(MIDI::Status::Value(lastStatus), buffer[i + 1], buffer[i + 2]);
							break;
						case MIDI::Type::Value::ProgramChange:
						case MIDI::Type::Value::AfterTouchChannel:
							for (int i = lPtr; i < rPtr; i = i + 1)
								this->parse(MIDI::Status::Value(lastStatus), buffer[i + 1]);
							break;
						default:
							break;
					}
				}
				//Point to next status
				lPtr = rPtr + 2;
				if (lPtr >= bufferSize) {
					//end of packet
					return;
				}
			}
		}
	};
}
}
