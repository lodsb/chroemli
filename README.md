# chroemli
Realtime MIDI note quantization/transposition (hardware)

Simple tool to quantize/transpose incoming midi notes from channels 1-15 to midi notes that are played on channel 16. 
Similar to a chord track in your DAW or a scale quantizer, except that you can change the scales/chords on the fly via a master keyboard or sequencer etc.

Let's call the notes on channels 1-15 *input-notes* and the ones arriving in channel 16 *chord-notes* for the remainder of the description.
Input notes are requantized as soon as the chord notes change, this means that held input notes will either continue if the chord includes the note or will be stopped and restarted with the new quantized pitch.

**chroemli** has two additional modes which are selectable via two switches:
  * scale velocity of notes that are to be quantized via chord input 
  * requantize all playing notes either immediately when the chord changes or when the next note on/off that is to be quantized arrives (reduces spurious notes that happen due to the serial nature of MIDI)

## Hardware

I used a vanilla Raspberry Pi Pico board and the Adafruit MIDI Featherwing, plus two switches and pull-down resistors (10k Ohm). You can also use the Adafruit RP2040 Board and get some colorful feedback when midi messages are sent via the inbuilt NeoPixel.
You can see/change the pin-assignment in [src/common.h](https://github.com/lodsb/chroemli/blob/master/src/common.h). The MIDI shield is connected to Serial1 and the switches to GPIO 7 & 8. You can also just omit the switches and be happy with the default behavior, which means immediate requantization and no velocity scaling.

Dirty Prototype:
![Alt text](doc/prototype.jpg?raw=true "Prototype")

Original Breadboard Version:
![Alt text](doc/breadboard.jpg?raw=true "Breadboard")

## Notes:
The requantization is done by first creating a diff between the currently quantized notes and the new quantization result, which is then used to
restart or continue currently playing notes. I did not optimize that part much since the RP2040 is quick enough to handle this exhaustive search in realtime.