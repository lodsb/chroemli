#pragma once

#include "ActiveNotesBuffer.h"

class NoteQuantizer
{
private:
   ActiveNotesBuffer  _buffer1;
   ActiveNotesBuffer  _buffer2;
   ActiveNotesBuffer* _current_buffer;

   NoteOnCallback  _note_on_callback;
   NoteOffCallback _note_off_callback;

   bool    _quantiziation_changed;
   bool    _active_quantizer_notes[NQUANTIZER_NOTES];
   uint8_t _active_quantizer_velocities[NQUANTIZER_NOTES];


public:
   NoteQuantizer(NoteOnCallback note_on_cb, NoteOffCallback note_off_cb)
            : _note_on_callback{note_on_cb}
            , _note_off_callback{note_off_cb}
            , _quantiziation_changed{false}
   {
      clear();
      _current_buffer = &_buffer1;
   }


   void activate_quantizer_note(const ActiveNotesBuffer& unquantized, bool apply_velocity_scaling, uint8_t note, uint8_t velo)
   {
      
      uint8_t note_in_octave = note % 12;
      _active_quantizer_notes[note_in_octave] = true;
      _active_quantizer_velocities[note_in_octave] = velo;

      _quantiziation_changed = true;

      ActiveNotesBuffer* work = get_work_buffer();
      work->clear();   

      if(apply_velocity_scaling)
      {
         requantize_buffer<true>(unquantized, *work);
      }
      else
      {
         requantize_buffer<false>(unquantized, *work);
      }

      _current_buffer->diff(*work, _note_on_callback, _note_off_callback);

      swap_buffers();      
   }

   void deactivate_quantizer_note(const ActiveNotesBuffer& unquantized, bool apply_velocity_scaling, uint8_t note)
   {
      uint8_t note_in_octave = note % 12;
      _active_quantizer_notes[note_in_octave] = false;

      _quantiziation_changed = true;

      ActiveNotesBuffer* work = get_work_buffer();
      work->clear();
      
      if(apply_velocity_scaling)
      {
         requantize_buffer<true>(unquantized, *work);
      }
      else
      {
         requantize_buffer<false>(unquantized, *work);
      }

      _current_buffer->diff(*work, _note_on_callback, _note_off_callback);

      swap_buffers();
   }

   void quantize_note( uint8_t note, uint8_t velocity, uint8_t& quantized_note, uint8_t& quantized_velocity)
   {
      uint8_t octave_offset  = (note / 12) * 12;
      uint8_t note_in_octave = note % 12;

      uint8_t min_dist     = 128;
      uint8_t min_dist_note_index= 0;

      bool is_quantized = false;

      for(uint8_t i = 0; i < NQUANTIZER_NOTES; i++)
      {
         if(_active_quantizer_notes[i])
         {
            uint8_t abs_dist = std::abs(note_in_octave - i);
            if(abs_dist <= min_dist)
            {
               min_dist = abs_dist;
               min_dist_note_index = i;
               is_quantized = true;
            }
         }
      }

      if(is_quantized) // we found a note to quantize to
      {
         quantized_note     = octave_offset + min_dist_note_index;
         quantized_velocity = _active_quantizer_velocities[min_dist_note_index];
      }
      else
      {
         quantized_note     = note;
         quantized_velocity = velocity;
      }

   }

   void clear()
   {
      for(bool& active_quantizer_note : _active_quantizer_notes)
      {
         active_quantizer_note = false;
      }
   }

   uint8_t num_active_quantizer_notes()
   {
      uint8_t count = 0;

      for(bool& active_quantizer_note : _active_quantizer_notes)
      {
         if(active_quantizer_note)
         {
            count++;
         }
      }

      return count;
   }

   uint8_t scale_velocity(uint8_t velo, uint8_t scaler)
   {
      uint16_t v16 = velo;
      uint16_t s16 = scaler;

      uint8_t result = ((v16*s16)/128) | 1;

      return result;
   }

   void add_note( uint8_t note, uint8_t velo, uint8_t channel, bool apply_velocity_scaling)
   {
      uint8_t qn, qv;
      quantize_note(note, velo, qn, qv);

      _current_buffer->activate_note(qn, velo, channel);

      if(apply_velocity_scaling)
      {

         _note_on_callback(qn, scale_velocity(velo, qv), channel);
      }
      else
      {
         _note_on_callback(qn, velo, channel);
      }
   }

   void remove_note( uint8_t note, uint8_t channel)
   {
      uint8_t qn, qv;
      quantize_note(note, 0, qn, qv);

      if(_current_buffer->is_note_active(qn, channel))
      {
         _current_buffer->deactivate_note(qn, channel);
         _note_off_callback(qn, channel);
      }
      else // probably missed quantizing that note
      {
         _note_off_callback(note, channel);
      }
   }

   ActiveNotesBuffer* get_work_buffer()
   {
      if( _current_buffer == &_buffer1)
      {
         return &_buffer2;
      }
      else
      {
         return &_buffer1;
      }
   }

   void swap_buffers()
   {
      if( _current_buffer == &_buffer1)
      {
         _current_buffer = &_buffer2;
      }
      else
      {
         _current_buffer = &_buffer1;
      }
   }

   void requantize_if_necessary(const ActiveNotesBuffer& unquantized, bool apply_velocity_scaling)
   {
      if(_quantiziation_changed)
      {
         requantize(unquantized, apply_velocity_scaling);

         _quantiziation_changed = false;
      }
   }

   void requantize(const ActiveNotesBuffer& unquantized, bool apply_velocity_scaling)
   {
      ActiveNotesBuffer* work = get_work_buffer();
      work->clear();

      if(apply_velocity_scaling)
      {
         requantize_buffer<true>(unquantized, *work);
      }
      else
      {
         requantize_buffer<false>(unquantized, *work);
      }

      _current_buffer->diff(*work, _note_on_callback, _note_off_callback);

      swap_buffers();
   }

   template<bool apply_velo_scaling>
   void requantize_buffer(const ActiveNotesBuffer& unquantized,
                          ActiveNotesBuffer& into)
   {
      into.clear();

      for(uint8_t i = 0; i < NNOTES; i++)
      {
         // optimization if any note is set in a channel
         if(unquantized.is_note_set_in_any_channel(i))
         {
            for(uint8_t c = 0; c < NCHANS; c++)
            {
               if(unquantized.is_note_active(i, c))
               {
                  uint8_t qn, qv;
                  uint8_t velo = unquantized.get_velocity(i, c);

                  quantize_note(i, velo, qn, qv);

                  if constexpr (apply_velo_scaling)
                  {
                     into.activate_note(qn, scale_velocity(velo, qv), c);
                  }
                  else
                  {
                     into.activate_note(qn, velo, c);
                  }
               }
            }
         }
      }
   }
};