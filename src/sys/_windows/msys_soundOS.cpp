//--------------------------------------------------------------------------//
// iq . 2003/2021 . code for 64 kb intros by RGBA                           //
//--------------------------------------------------------------------------//

#define WIN32_LEAN_AND_MEAN
#define WIN32_EXTRA_LEAN
#include <windows.h>
#include <mmsystem.h>
#include <mmreg.h>

#define PLAYER_RATE         48000
#define PLAYER_NUMCHANNELS  2
#define PLAYER_LATENCY      20

//------------------------------------------------------------------------------------------------------------

static const WAVEFORMATEX wavinfo = {
    WAVE_FORMAT_IEEE_FLOAT,             // Use IEEE float format
    PLAYER_NUMCHANNELS,                 // number of channels
    PLAYER_RATE,                        // sample rate
    PLAYER_RATE * PLAYER_NUMCHANNELS * sizeof(float),  // avg bytes per second
    PLAYER_NUMCHANNELS * sizeof(float), // block align (bytes per frame)
    32,                                 // bits per sample
    0
};

static HWAVEOUT h   =   0;      // this one is anty-thread-safe, but we don't care in this case...
static WAVEHDR  wav = { 0, 0, 0,0, WHDR_BEGINLOOP|WHDR_ENDLOOP, 0xffffffff, 0,0 };


//-----------------------------------------------------------------------------


int msys_soundInit( void *buffer, int totallengthinbytes )
{
    wav.dwBufferLength = totallengthinbytes;
    wav.lpData = (char*)buffer;

    if( waveOutOpen( &h, WAVE_MAPPER, &wavinfo, 0, 0, 0) )
        return( 0 );
    waveOutPrepareHeader( h, &wav, sizeof(WAVEHDR) );

    return( 1 );
}

void msys_soundStart( void )
{
    waveOutWrite( h, &wav, sizeof(WAVEHDR) );
}

void msys_soundEnd( void )
{
    waveOutReset( h );
    waveOutClose( h );
}

long msys_soundGetPosInSamples( void )
{
    MMTIME  mmt;

    mmt.wType = TIME_SAMPLES;
    waveOutGetPosition( h, &mmt, sizeof(MMTIME) );

	return mmt.u.cb;
}
