#pragma once

// use std::vector for tools
#ifdef TOOLS
#define MSYS_USE_STD
#endif

#ifdef MSYS_USE_STD
#include <vector>
 
template<class T>
using Vec = std::vector<T>;

#else
#include "msys_assert.h"
//#include <stdlib.h>
#include <initializer_list>
#include "msys_libc.h"

void* bumpAlloc(int size);

template<typename T>
struct Vec
{
    unsigned int                 Size;
    unsigned int                 Capacity;
    T*                  Data;

   // (void*, unsigned int, unsigned int, void(__thiscall*)(void*))
    // Provide standard typedefs but we don't use them ourselves.
    typedef T                   value_type;
    typedef value_type*         iterator;
    typedef const value_type*   const_iterator;

    // Constructors, destructor
    Vec() noexcept { Size = Capacity = 0; Data = NULL; }

    Vec<T>& operator=(const Vec<T>& src) noexcept
    { 
        clear(); resize(src.Size); if (src.Data) msys_memcpy(Data, src.Data, (size_t)Size * sizeof(T)); return *this;
    }
    
    Vec(Vec<T>&& src) noexcept {
        Size = src.Size;
        Capacity = src.Capacity;
        Data = src.Data;
        src.Capacity = 0;
        src.Size = 0;
        src.Data = 0;
    }
    
    //~Vec() noexcept { /*if (Data) free(Data);*/ } // Important: does not destruct anything

    void         clear() noexcept { if (Data) { Size = Capacity = 0; /*free(Data);*/ Data = NULL; } }  // Important: does not destruct anything

    bool         empty() const noexcept { return Size == 0; }
    unsigned int          size() const noexcept { return Size; }
    T&           operator[](unsigned int i) noexcept { assert(i < Size); return Data[i]; }
    const T&     operator[](unsigned int i) const noexcept { assert(i < Size); return Data[i]; }

    T* begin() noexcept { return Data; }
    const T*     begin() const noexcept { return Data; }
    T*           end() noexcept { return Data + Size; }
    const T*     end() const noexcept { return Data + Size; }
    T&           back() noexcept { assert(Size > 0); return Data[Size - 1]; }
    const T&     back() const noexcept { assert(Size > 0); return Data[Size - 1]; }
    //inline void         swap(Vec<T>& rhs)              { unsigned int rhs_size = rhs.Size; rhs.Size = Size; Size = rhs_size; unsigned int rhs_cap = rhs.Capacity; rhs.Capacity = Capacity; Capacity = rhs_cap; T* rhs_data = rhs.Data; rhs.Data = Data; Data = rhs_data; }

    unsigned int          _grow_capacity(int sz) const noexcept { int new_capacity = Capacity ? (Capacity + Capacity) : 8; return new_capacity > sz ? new_capacity : sz; }
    void         resize(unsigned int new_size) noexcept { if (new_size > Capacity) reserve(_grow_capacity(new_size)); Size = new_size; }
    void         resize(unsigned int new_size, const T& v) noexcept { if (new_size > Capacity) reserve(_grow_capacity(new_size)); if (new_size > Size) for (unsigned int n = Size; n < new_size; n++) msys_memcpy(&Data[n], &v, sizeof(v)); Size = new_size; }
    void         reserve(unsigned int new_capacity) noexcept { if (new_capacity <= Capacity) return; T* new_data = (T*)bumpAlloc((size_t)new_capacity * sizeof(T)); if (Data) { msys_memcpy(new_data, Data, (size_t)Size * sizeof(T)); /*free(Data);*/ } Data = new_data; Capacity = new_capacity; }
    
    // NB: It is illegal to call push_back/push_front/insert with a reference pointing inside the ImVector data itself! e.g. v.push_back(v[10]) is forbidden.
    void         push_back(const T& v) noexcept
    {
        if (Size == Capacity) reserve(_grow_capacity(Size + 1)); msys_memcpy(&Data[Size], &v, sizeof(v)); Size++;
    }

    T* data() noexcept { return Data; }
    Vec(const Vec<T>& src) noexcept {
        Size = Capacity = 0; Data = NULL; operator=(src);
    }
    Vec(std::initializer_list<T> values) noexcept : Size(int(values.size())) {
        Data = (T*)bumpAlloc((size_t)Size * sizeof(T));
        Capacity = Size;
        long i = 0;
        for (const auto value : values) {
            Data[i++] = value;
        }
    }
private:


};
#endif
