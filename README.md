# chroemli
Realtime MIDI note quantization (hardware)

Simple tool to quantize incoming midi notes from channels 1-15 to midi notes that are played on channel 16. Similar to a chord track in your DAW or a scale quantizer, except that you can change the scales/chords on the fly via a master keyboard or sequencer etc.

Let's call the notes on channels 1-15 *input-notes* and the ones arriving in channel 16 *chord-notes* for the raminder of the description.
Input notes are requantized as soon as the choord notes change, this means that held input notes will either continue if the chord includes the note or will stop and restarted with the new quantized pitch.
**Chroemli** has two additional modes which are selectable via two switches:
  * scale velocity of notes that are to be quantized via chord input 
  * requantize immediately all playing notes either when the chord changes or when the next note on/off that is to be quantized arrives (reduces spurious notes that happen due to the serial nature of MIDI)

I used a vanilla Raspberry Pi Pico board plus the Adafruit MIDI Featherwing, plus two switches and pull-down resistors (10k Ohm). You can also use the Adafruit RP2040 Board and get some colorful feedback when midi messages are sent via the inbuilt NeoPixel.
