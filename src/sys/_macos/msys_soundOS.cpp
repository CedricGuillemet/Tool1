#include <AudioToolbox/AudioToolbox.h>
#include <stdint.h>
#include <string.h>

#define PLAYER_RATE         48000
#define PLAYER_NUMCHANNELS  2
#define PLAYER_LATENCY_MS   20
#define BUFFER_COUNT        3

static AudioQueueRef gQueue = NULL;
static AudioQueueBufferRef gBuffers[BUFFER_COUNT];

static const uint8_t *gAudioBytes = NULL;   // byte-wise view of user buffer
static size_t gAudioByteLen = 0;            // total bytes available
static size_t gAudioBytePos = 0;            // current byte position
static UInt32 gBytesPerChunk = 0;           // target bytes per queued buffer
static int gStopped = 0;

static void AQCallback(void *userData, AudioQueueRef q, AudioQueueBufferRef b)
{
    if (gStopped || gAudioBytePos >= gAudioByteLen) {
        // Reached end: stop queue and do NOT re-enqueue this buffer.
        AudioQueueStop(q, false);
        gStopped = 1;
        return;
    }

    size_t remaining = gAudioByteLen - gAudioBytePos;
    UInt32 toCopy = (UInt32)((remaining < gBytesPerChunk) ? remaining : gBytesPerChunk);

    memcpy(b->mAudioData, gAudioBytes + gAudioBytePos, toCopy);
    b->mAudioDataByteSize = toCopy;
    gAudioBytePos += toCopy;

    AudioQueueEnqueueBuffer(q, b, 0, NULL);

    // If we just consumed the last bytes, request a stop so the queue winds down.
    if (gAudioBytePos >= gAudioByteLen) {
        AudioQueueStop(q, false);
        gStopped = 1;
    }
}

int msys_soundInit(void *buffer, int totallengthinbytes)
{
    // Guard: negative int promoted to huge size_t will cause “infinite” playback.
    if (totallengthinbytes <= 0 || buffer == NULL) return 0;

    gAudioBytes   = (const uint8_t*)buffer;  // buffer holds interleaved float samples
    gAudioByteLen = (size_t)totallengthinbytes;
    gAudioBytePos = 0;
    gStopped      = 0;

    AudioStreamBasicDescription asbd;
    memset(&asbd, 0, sizeof(asbd));
    asbd.mSampleRate       = PLAYER_RATE;
    asbd.mFormatID         = kAudioFormatLinearPCM;
    asbd.mFormatFlags      = kAudioFormatFlagIsFloat | kAudioFormatFlagIsPacked
#if __BIG_ENDIAN__
                           | kAudioFormatFlagIsBigEndian
#endif
                           ;
    asbd.mBitsPerChannel   = 32;
    asbd.mChannelsPerFrame = PLAYER_NUMCHANNELS;     // 2
    asbd.mFramesPerPacket  = 1;
    asbd.mBytesPerFrame    = PLAYER_NUMCHANNELS * sizeof(float); // 8 bytes/frame
    asbd.mBytesPerPacket   = asbd.mBytesPerFrame;

    OSStatus st = AudioQueueNewOutput(&asbd, AQCallback, NULL, NULL, kCFRunLoopCommonModes, 0, &gQueue);
    if (st != noErr) return 0;

    // 44100 * 2ch * 4B * (latency ms / 1000)
    gBytesPerChunk = (UInt32)((PLAYER_RATE * PLAYER_NUMCHANNELS * sizeof(float) * PLAYER_LATENCY_MS) / 1000);

    // Safety: never allocate buffers larger than the whole payload.
    if (gBytesPerChunk == 0 || gBytesPerChunk > gAudioByteLen) {
        gBytesPerChunk = (UInt32)gAudioByteLen;
    }

    for (int i = 0; i < BUFFER_COUNT; ++i) {
        AudioQueueAllocateBuffer(gQueue, gBytesPerChunk, &gBuffers[i]);
        // Prime the queue with initial data.
        AQCallback(NULL, gQueue, gBuffers[i]);
        if (gStopped) break; // In case the file is shorter than one buffer.
    }

    return 1;
}

void msys_soundStart(void)
{
    if (gQueue && !gStopped) {
        AudioQueueStart(gQueue, NULL);
    }
}

void msys_soundEnd(void)
{
    if (gQueue) {
        AudioQueueStop(gQueue, true);
        AudioQueueDispose(gQueue, true);
        gQueue = NULL;
    }
    gAudioBytes = NULL;
    gAudioByteLen = gAudioBytePos = 0;
    gStopped = 0;
}

long msys_soundGetPosInSamples(void)
{
    // Convert byte position to frames: bytes / (ch * sizeof(float))
    size_t frames = gAudioBytePos / (PLAYER_NUMCHANNELS * sizeof(float));
    return (long)frames;
}
