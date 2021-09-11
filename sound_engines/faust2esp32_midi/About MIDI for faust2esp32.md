# About MIDI for faust2esp32

In the 2020 paper by Michon & Letz et al polyphony for ESP32 was reported using a TTGO TAudio board.

On my request, Stephane Letz gave the following instructionss for reproducing those results.

Use: **faustesp32 -midi nvoices num ....**  for generation of the sound engine

I myself found [this information](https://faustdoc.grame.fr/manual/midi/) on MIDI and Polyphony Support  
  
 
Input by S. Letz:    

- If you use a MIDI device, MIDI events are  directly received and handled.

- MIDI code is activated here: https://github.com/grame-cncm/faust/blob/master-dev/architecture/esp32/esp32.cpp#L111

- Low-level MIDI handler is https://github.com/grame-cncm/faust/blob/master-dev/architecture/faust/midi/esp32-midi.h

- Read here to get an general overview: https://faustdoc.grame.fr/manual/architectures/ 


Walkthrough for generation of MIDI controlled polyphony:

- Creat the sound engine  
  - this
  - that
- Read info supplied with the compiled sound engine  
- Apply this in an app (esp_idf,  arduino) 
  
  
