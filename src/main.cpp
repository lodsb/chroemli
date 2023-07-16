#include "common.h"
#include "ActiveNotesBuffer.h"
#include "NoteQuantizer.h"


Adafruit_NeoPixel neo_pixel = Adafruit_NeoPixel(1, NEO_PIXEL_PIN, NEO_GRB + NEO_KHZ800);

////////////////////////////////////////////////////////////////////////////////
void setup_neo_pixel()
{
   neo_pixel.begin();
   neo_pixel.setBrightness(50);
   neo_pixel.show();
}

void set_neo_pixel(uint8_t r, uint8_t g, uint8_t b)
{
   neo_pixel.setPixelColor(0, Adafruit_NeoPixel::Color(r, g, b));
   neo_pixel.show();
}

void set_neo_pixel_off()
{
   neo_pixel.setPixelColor(0, Adafruit_NeoPixel::Color(0, 0, 0));
   neo_pixel.show();
}

////////////////////////////////////////////////////////////////////////////////

void setup_switches()
{
   pinMode(SWITCH_ONE_PIN, INPUT);
   pinMode(SWITCH_TWO_PIN, INPUT);
}

bool is_switch_one_on()
{
   return digitalRead(SWITCH_ONE_PIN) == HIGH;
}

bool is_switch_two_on()
{
   return digitalRead(SWITCH_TWO_PIN) == HIGH;
}

////////////////////////////////////////////////////////////////////////////////

void setup_led()
{
   pinMode(LED_BUILTIN, OUTPUT);
}

void set_board_led_on()
{
   digitalWrite(LED_BUILTIN, HIGH);
}

void set_board_led_off()
{
   digitalWrite(LED_BUILTIN, LOW);
}



////////////////////////////////////////////////////////////////////////////////

MIDI_CREATE_INSTANCE(HardwareSerial, MIDI_IO_UART,  MIDI);

////////////////////////////////////////////////////////////////////////////////

void send_note_on(uint8_t note, uint8_t velocity, uint8_t channel)
{
   MIDI.sendNoteOn(note, velocity, channel);
}

void send_note_off(uint8_t note, uint8_t channel)
{
   MIDI.sendNoteOff(note, 0, channel);
}

////////////////////////////////////////////////////////////////////////////////

ActiveNotesBuffer active_input_notes_unquantized;
NoteQuantizer note_quantizer{&send_note_on, &send_note_off};

void handle_note_on(byte channel, byte note, byte velocity)
{
   set_neo_pixel(0, 25, 0);

   bool requantize_on_note_on  = is_switch_one_on();
   bool apply_velocity_scaling = is_switch_two_on();

   if(channel != 16)
   {
      active_input_notes_unquantized.activate_note(note, velocity, channel);
      note_quantizer.add_note(note, velocity, channel, apply_velocity_scaling);

      if(requantize_on_note_on)
      {
         note_quantizer.requantize_if_necessary(active_input_notes_unquantized, apply_velocity_scaling);
      }
   }
   else
   {
      note_quantizer.activate_quantizer_note(note, velocity);

      if(!requantize_on_note_on)
      {
         note_quantizer.requantize_if_necessary(active_input_notes_unquantized, apply_velocity_scaling);
      }
   }
}

void handle_note_off(byte channel, byte note, byte velocity)
{
   set_neo_pixel(25, 0, 0);

   bool requantize_on_note_off = is_switch_one_on();
   bool apply_velocity_scaling = is_switch_two_on();

   if(channel != 16)
   {
      active_input_notes_unquantized.deactivate_note(note, channel);
      note_quantizer.remove_note(note, channel);

      if(requantize_on_note_off)
      {
         note_quantizer.requantize_if_necessary(active_input_notes_unquantized, apply_velocity_scaling);
      }
   }
   else
   {
      note_quantizer.deactivate_quantizer_note(note);

      if(!requantize_on_note_off)
      {
         note_quantizer.requantize_if_necessary(active_input_notes_unquantized, apply_velocity_scaling);
      }
   }
}

void handle_after_touch_poly(byte channel, byte note, byte pressure)
{
   MIDI.sendAfterTouch(note, pressure, channel);
}

void handle_control_change(byte channel, byte number, byte value)
{
   MIDI.sendControlChange(number, value, channel);
}

void handle_program_change(byte channel, byte number)
{
   MIDI.sendProgramChange(number, channel);
}

void handle_after_touch_channel(byte channel, byte pressure)
{
   MIDI.sendAfterTouch(pressure, channel);
}

void handle_pitch_bend(byte channel, int bend)
{
   MIDI.sendPitchBend(bend, channel);
}

void handle_sysex(byte* array, unsigned size)
{
   MIDI.sendSysEx(size, array);
}

void handle_time_code_quarter_frame(byte data)
{
   MIDI.sendTimeCodeQuarterFrame(data);
}

void handle_song_position(unsigned int beats)
{
   MIDI.sendSongPosition(beats);
}

void handle_song_select(byte songnumber)
{
   MIDI.sendSongSelect(songnumber);
}

void handle_tune_request()
{
   MIDI.sendTuneRequest();
}

void handle_clock()
{
   MIDI.sendClock();
}

void handle_start()
{
   MIDI.sendStart();
}

void handle_continue()
{
   MIDI.sendContinue();
}

void handle_stop()
{
   MIDI.sendStop();
}

void handle_active_sensing()
{
   MIDI.sendActiveSensing();
}

void handle_system_reset()
{
   MIDI.sendSystemReset();
}

////////////////////////////////////////////////////////////////////////////////

void setup_midi()
{
   MIDI.setHandleNoteOn(handle_note_on);
   MIDI.setHandleNoteOff(handle_note_off);

   MIDI.setHandleAfterTouchPoly(handle_after_touch_poly);
   MIDI.setHandleControlChange(handle_control_change);
   MIDI.setHandleProgramChange(handle_program_change);
   MIDI.setHandleAfterTouchChannel(handle_after_touch_channel);
   MIDI.setHandlePitchBend(handle_pitch_bend);
   MIDI.setHandleSystemExclusive(handle_sysex);
   MIDI.setHandleTimeCodeQuarterFrame(handle_time_code_quarter_frame);
   MIDI.setHandleSongPosition(handle_song_position);
   MIDI.setHandleSongSelect(handle_song_select);
   MIDI.setHandleTuneRequest(handle_tune_request);
   MIDI.setHandleClock(handle_clock);
   MIDI.setHandleStart(handle_start);
   MIDI.setHandleContinue(handle_continue);
   MIDI.setHandleStop(handle_stop);
   MIDI.setHandleActiveSensing(handle_active_sensing);
   MIDI.setHandleSystemReset(handle_system_reset);


   MIDI.turnThruOff();
   MIDI.begin(MIDI_CHANNEL_OMNI);
   MIDI.turnThruOff();
}

////////////////////////////////////////////////////////////////////////////////

void setup() {
   setup_switches();
   setup_led();
   setup_neo_pixel();
   setup_midi();
}

////////////////////////////////////////////////////////////////////////////////

void loop()
{
   if (MIDI.read())
   {
   }
   else
   {
      set_neo_pixel(0, 0, 0);
   }

   if(is_switch_one_on() || is_switch_two_on())
   {
      set_board_led_on();
   }
   else
   {
      set_board_led_off();
   }
}