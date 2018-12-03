/*
 * HashMap container for uint256 index -> value
 * Part of RngCoin project.
 *
 */

#ifndef RNG_UINT256HM_H
#define RNG_UINT256HM_H

#include "uint256.h"
#include "random.h"

// Container - debug version
template<class DATA>
  class uint256HashMap {
public:
    struct Data {
	Data() : next(~0) {}
	DATA     value;
	uint256  key;
	uint32_t next; // ~0 = EMPTY, END = m_mask + 1
    };

   uint256HashMap() : m_randseed((uint32_t)GetRand(1 << 30)), m_data(NULL) {};
  ~uint256HashMap() { Set(0); };

  void Set(uint32_t size) {
    // delete data if exist and needed
    if(size == 0) {
      delete [] m_data;
      m_data = NULL;
      return;
    }
    // Ignore 2nd Set() [reenter]
    if(m_data)
      return;

    if(size > (1 << 29))
      size = 1 << 29;

    // compute size 2^n
    for(m_mask = 8; m_mask < size; m_mask <<= 1);

    //LogPrintf("uint256HashMap:Set(%u/%u) data=%u sz=%u\n", size, m_mask, (unsigned)sizeof(struct Data), (unsigned)(m_mask * sizeof(struct Data)));
    // allocate memory
    m_data = new Data[m_head = m_tail = m_mask];
    m_used = m_deleted = 0;
    // set real mask
    m_mask--;
  } // Init

  uint32_t size() const {
      return m_used - m_deleted;
  }

  void clear() { // cleanup hashtable, no delete memory
    if(m_deleted)
      for(uint32_t i = 0; i <= m_mask; i++)
        m_data[i].next = ~0; // mark as free
    else
      while(m_head <= m_mask) {
        uint32_t nxt = m_data[m_head].next & 0x7fffffff;
         m_data[m_head].next = ~0; // mark as free
         m_head = nxt;
      }
    m_used = m_deleted = 0;
    m_tail = m_head = m_mask + 1; // END
  }

  Data *Search(const uint256 &key) const {
    Data *rc = Lookup(key, ~0);
    return ((rc->next) & 0x80000000)? NULL : rc;
  } // Search


  Data *First() const {
    return m_head > m_mask? NULL : m_data + m_head;
  }


  Data *Next(Data *cur) const {
     do {
       if((cur->next) == m_mask + 1)
         return NULL; // EOL
       cur = m_data + (cur->next & 0x7fffffff);
     } while(((int32_t)cur->next) < 0);
     return cur;
  } // Next

  // Mark for delete at next rehash
  void MarkDel(Data *cur) {
     if((cur->next & 0x80000000) == 0) {
       cur->next |= 0x80000000;
       m_deleted++;
       while(m_data[m_head].next & 0x80000000)
         m_head = m_data[m_head].next ^ 0x80000000;
       if(m_head > m_mask)
         m_tail = m_head;
     }
  }

  Data *Insert(const uint256 &key, DATA &value) {
    uint32_t newsz = 0;
    if(m_deleted > m_mask >> 1)
      newsz = m_mask;
    if(m_used >= m_mask - (m_mask >> 2))
      newsz = m_mask << 1;
    if(newsz) {
      // rehash to 2xN table
      uint256HashMap<DATA> rehashed;
      rehashed.Set(newsz);
      Data *p;
      for(p = First(); p != NULL; p = Next(p))
        rehashed.Insert(p->key, p->value);
      p = m_data;
      m_data    = rehashed.m_data;
      m_used    = rehashed.m_used;
      m_deleted = 0;
      m_mask    = rehashed.m_mask;
      m_head    = rehashed.m_head;
      m_tail    = rehashed.m_tail;
      m_randseed = rehashed.m_randseed;
      rehashed.m_data = p; // release current buffer
    } // reahsh

    Data *p = Lookup(key, 1 << 29);
    if(p->next == (uint32_t)~0) { // empty cell
      m_used++;
      p->next = m_mask + 1;
      if(m_head > m_mask)
         m_head = m_tail = p - m_data;
      else {
        m_data[m_tail].next = p - m_data;
        m_tail = p - m_data;
      }
    }
    if(p->next & 0x80000000) {
      p->next ^= 0x80000000; // Clear MarkDel
      m_deleted--; // Reused deleted cell
    }
    p->key   = key;
    p->value = value;
    return p;
  } // Insert

private:

  // Return pointer to the found cell, or to an empty cell
  Data *Lookup(const uint256 &key, uint32_t stop) const {
      const uint32_t *p = ((base_blob<256>*)&key)->GetDataPtr();
      // Lowest part left; if changed, need modify indexes
      uint32_t pos  = p[0];
      uint32_t step = (p[1] ^ (p[2] + m_randseed)) | 1; // odd step
      Data *rc;
      do {
	pos = (pos + step) & m_mask;
	rc = m_data + pos;
      } while((rc->next) < stop && memcmp(p, ((base_blob<256>*)&(rc->key))->GetDataPtr(), 256 / 8));
      return rc;
  } // Lookup

  uint32_t  m_head, m_tail;
  uint32_t  m_mask;
  uint32_t  m_used, m_deleted;
  uint32_t  m_randseed;
  Data     *m_data;

}; // uint256HashMap

#endif
