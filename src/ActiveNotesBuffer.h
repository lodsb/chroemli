#pragma once

#include "common.h"

class ActiveNotesBuffer
{

private:

   int8_t  _active_notes[NNOTES][NCHANS]{};
   int8_t  _is_active_sum[NNOTES]{};
   uint8_t _velocity[NNOTES][NCHANS]{};

public:

   ActiveNotesBuffer()
   {
      clear();
   }

   void activate_note( uint8_t note, uint8_t velo, uint8_t channel)
   {
      _active_notes[note][channel] = _active_notes[note][channel] + 1;
      _is_active_sum[note]      = _is_active_sum[note] + 1;
      _velocity[note][channel]  = velo;
   }

   void deactivate_note(uint8_t note, uint8_t channel)
   {
      _active_notes[note][channel] = std::max(_active_notes[note][channel] - 1, 0);
      _is_active_sum[note]         = std::max(_is_active_sum[note] - 1, 0);
   }

   bool is_note_set_in_any_channel(uint8_t note) const
   {
      return _is_active_sum[note] > 0;
   }

   bool is_note_active(uint8_t note, uint8_t channel) const
   {
      return _active_notes[note][channel] > 0;
   }

   uint8_t get_velocity(uint8_t note, uint8_t channel) const
   {
      return _velocity[note][channel];
   }

   void clear()
   {
      for(uint8_t i = 0; i < NNOTES; i++)
      {
         for(uint8_t c = 0; c < NCHANS; c++)
         {
            _active_notes[i][c] = 0;
         }

         _is_active_sum[i] = 0;
      }
   }

   // triggers a note on/off depending on whether notes are active in the
   // other buffer
   void diff(const ActiveNotesBuffer&  other,
             NoteOnCallback            note_on_cb,
             NoteOffCallback           note_off_cb)
   {
      for(uint8_t i = 0; i < NNOTES; i++)
      {
         // optimization if any note is set in a channel
         //if(is_active_sum[i] != other.is_active_sum[i])
         {
            for(uint8_t c = 0; c < NCHANS; c++)
            {
               int8_t difference = _active_notes[i][c] - other._active_notes[i][c];

               if(difference > 0)
               {
                  for(int8_t d = 0; d < difference; d++)
                  {
                     note_off_cb(i, c);
                  }
               }
               else if(difference < 0)
               {
                  for(int8_t d = 0; d < -difference; d++)
                  {
                     note_on_cb(i, other._velocity[i][c], c);
                  }                  
               }               
            }
         }
      }
   }

   void copy_to(ActiveNotesBuffer& other)
   {
      memcpy(other._active_notes    , _active_notes, sizeof(_active_notes));
      memcpy(other._is_active_sum, _is_active_sum, sizeof(_is_active_sum));
      memcpy(other._velocity, _velocity, sizeof(_velocity));
   }
};