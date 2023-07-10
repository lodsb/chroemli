#pragma once

#include "common.h"

class ActiveNotesBuffer
{
public:

   bool    m_is_active[NNOTES][NCHANS]{};
   uint8_t m_is_active_sum[NNOTES]{};
   uint8_t m_velocity[NNOTES][NCHANS]{};

   ActiveNotesBuffer()
   {
      clear();
   }

   void activate_note( uint8_t note, uint8_t velo, uint8_t channel)
   {
      m_is_active[note][channel] = true;
      m_is_active_sum[note]= m_is_active_sum[note] + 1;
      m_velocity[note][channel]  = velo;
   }

   void deactivate_note(uint8_t note, uint8_t channel)
   {
      m_is_active[note][channel] = false;
      m_is_active_sum[note]= m_is_active_sum[note] - 1;
   }


   void clear()
   {
      for(uint8_t i = 0; i < NNOTES; i++)
      {
         for(uint8_t c = 0; c < NCHANS; c++)
         {
            m_is_active[i][c] = false;
         }

         m_is_active_sum[i] = 0;
      }
   }

   // triggers a note on/off depending on whether notes are active in the
   // other buffer
   void diff(const ActiveNotesBuffer& other,
             NoteOnCallback note_on_cb,
             NoteOffCallback note_off_cb)
   {
      for(uint8_t i = 0; i < NNOTES; i++)
      {
         // optimization if any note is set in a channel
         //if(is_active_sum[i] != other.is_active_sum[i])
         {
            for(uint8_t c = 0; c < NCHANS; c++)
            {
               if( m_is_active[i][c] && !other.m_is_active[i][c])
               {
                  note_off_cb(i, c);
               }
               else if( !m_is_active[i][c] && other.m_is_active[i][c])
               {
                  note_on_cb(i, other.m_velocity[i][c], c);
               }
            }
         }
      }
   }

   void copy_to(ActiveNotesBuffer& other)
   {
      memcpy(other.m_is_active    , m_is_active, sizeof(m_is_active));
      memcpy(other.m_is_active_sum, m_is_active_sum, sizeof(m_is_active_sum));
      memcpy(other.m_velocity, m_velocity, sizeof(m_velocity));
   }
};