//--------------------------------------------------------------------------//
// iq . 2003/2021 . code for 64 kb intros by RGBA                           //
//--------------------------------------------------------------------------//

#ifndef _INTRO_H_
#define _INTRO_H_

struct FrameBuffer;
int intro_init();
int intro_do(void);
bool RenderFrame(int frame, FrameBuffer* contentViewFrameBuffer);

#endif
