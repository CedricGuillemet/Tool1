//--------------------------------------------------------------------------//
// iq . 2003/2021 . code for 64 kb intros by RGBA                           //
//--------------------------------------------------------------------------//

#ifndef _MSYS_H_
#define _MSYS_H_

#include "msys_types.h"
#include "msys_libc.h"
#include "msys_random.h"
#include "msys_sound.h"
#include "msys_timer.h"
#include "msys_thread.h"
#include "msys_glext.h"
#include "msys_debug.h"
#include "msys_vector.h"
#include "msys_glutils.h"

int  msys_init( uint64 h );
void msys_end( void );

#endif

