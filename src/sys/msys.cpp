//--------------------------------------------------------------------------//
// iq . 2003/2021 . code for 64 kb intros by RGBA                           //
//--------------------------------------------------------------------------//

#include "msys_glext.h"
#include "msys.h"

int msys_init( uint64 h )
{
    if( !msys_glextInit() ) return 0;

    return 1;
}

void msys_end( void )
{
}
