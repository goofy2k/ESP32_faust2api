# MIDIBLE

A MIDI over BLE interface for esp-idf.

Implementation of MIDI parser is [Arduino-BLE-MIDI](https://github.com/lathoub/Arduino-BLE-MIDI).

## Requirements
- esp-idf v3.1.4

## Dependencies

- [cpp_util](https://github.com/nkolban/esp32-snippets/tree/master/cpp_utils)

## Usage

Like this.

```
MIDIBLE::BLEInterface *interface = new MIDIBLE::BLEInterface(name, udid);

interface->note_on_handler = [&](MIDIBLE::MIDI::Channel channel, uint8_t note, uint8_t velocity) {
 // note on
};

interface->note_off_handler = [&](MIDIBLE::MIDI::Channel channel, uint8_t note, uint8_t velocity) {
 // note off
};
```
