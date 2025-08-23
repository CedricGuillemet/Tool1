//--------------------------------------------------------------------------//
// iq . 2003 . code for the Paradise 64 kb intro by RGBA                    //
//--------------------------------------------------------------------------//

#include <stdlib.h>
#include <string.h>
#ifdef DEBUG
#include <stdio.h>
#endif
#include "../msys_types.h"


//-----------------------------------------------------------------------------


void *msys_mallocAlloc( uint32 amount )
{
    void *ptr;

    ptr = malloc( amount );

    if( ptr )
        memset( ptr, 0, amount );

    return( ptr );
}

void msys_mallocFree( void *ptr )
{
    free( ptr );
}



