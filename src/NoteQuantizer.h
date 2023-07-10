#pragma once

#include "ActiveNotesBuffer.h"

class NoteQuantizer
{
private:
   ActiveNotesBuffer m_buffer1;
   ActiveNotesBuffer m_buffer2;

   ActiveNotesBuffer* m_current_buffer;

   NoteOnCallback  m_note_on_callback;
   NoteOffCallback m_note_off_callback;

   bool    m_quantiziation_changed;
   bool    m_active_quantizer_notes[NQUANTIZER_NOTES];
   uint8_t m_active_quantizer_velocities[NQUANTIZER_NOTES];


public:
   NoteQuantizer(NoteOnCallback note_on_cb, NoteOffCallback note_off_cb)
           : m_note_on_callback{note_on_cb}
             , m_note_off_callback{note_off_cb}
             , m_quantiziation_changed{false}
   {
      clear();
      m_current_buffer = &m_buffer1;
   }


   void activate_quantizer_note(uint8_t note, uint8_t velo)
   {
      uint8_t note_in_octave = note % 12;
      m_active_quantizer_notes[note_in_octave] = true;
      m_active_quantizer_velocities[note_in_octave] = velo;

      m_quantiziation_changed = true;
   }

   void deactivate_quantizer_note( uint8_t note)
   {
      uint8_t note_in_octave = note % 12;
      m_active_quantizer_notes[note_in_octave] = false;

      m_quantiziation_changed = true;
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
         if(m_active_quantizer_notes[i])
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
         quantized_velocity = m_active_quantizer_velocities[min_dist_note_index];
      }
      else
      {
         quantized_note = note;
         quantized_velocity = velocity;
      }

   }

   void clear()
   {
      for(bool & active_quantizer_note : m_active_quantizer_notes)
      {
         active_quantizer_note = false;
      }
   }

   uint8_t num_active_quantizer_notes()
   {
      uint8_t count = 0;

      for(bool active_quantizer_note : m_active_quantizer_notes)
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

      m_current_buffer->activate_note(qn, velo, channel);

      if(apply_velocity_scaling)
      {

         m_note_on_callback(qn, scale_velocity(velo, qv), channel);
      }
      else
      {
         m_note_on_callback(qn, velo, channel);
      }
   }

   void remove_note( uint8_t note, uint8_t channel)
   {
      uint8_t qn, qv;
      quantize_note(note, 0, qn, qv);

      m_current_buffer->deactivate_note(qn, channel);
      m_note_off_callback(qn, channel);
   }

   ActiveNotesBuffer* get_work_buffer()
   {
      if( m_current_buffer == &m_buffer1)
      {
         return &m_buffer2;
      }
      else
      {
         return &m_buffer1;
      }
   }

   void swap_buffers()
   {
      if( m_current_buffer == &m_buffer1)
      {
         m_current_buffer = &m_buffer2;
      }
      else
      {
         m_current_buffer = &m_buffer1;
      }
   }

   void requantize_if_necessary(const ActiveNotesBuffer& unquantized, bool apply_velocity_scaling)
   {
      if(m_quantiziation_changed)
      {
         requantize(unquantized, apply_velocity_scaling);

         m_quantiziation_changed = false;
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

      m_current_buffer->diff(*work, m_note_on_callback, m_note_off_callback);

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
         if( unquantized.m_is_active_sum[i] != 0)
         {
            for(uint8_t c = 0; c < NCHANS; c++)
            {
               if(unquantized.m_is_active[i][c])
               {
                  uint8_t qn, qv;
                  uint8_t velo = unquantized.m_velocity[i][c];

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