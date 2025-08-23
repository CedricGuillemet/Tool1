//--------------------------------------------------------------------------//
// iq . 2003/2021 . code for 64 kb intros by RGBA                           //
//--------------------------------------------------------------------------//

#define WIN32_LEAN_AND_MEAN
#define WIN32_EXTRA_LEAN
#include <windows.h>

#include "../msys_thread.h"

void * msys_threadNew( THREAD_FUNC func )
{
	unsigned long thid;

    HANDLE th = CreateThread( NULL, 0, (LPTHREAD_START_ROUTINE)func, 0,0, &thid );

    SetThreadPriority( th, THREAD_PRIORITY_HIGHEST );

    return (void *)th;
}



void msys_threadDelete( void * id )
{
    CloseHandle( (HANDLE)id );
}
