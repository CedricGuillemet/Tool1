#pragma once
#include "msys_vector.h"
#include "EditUtils.h"
#include "TextEditor.h"

class TextEdit : public EditBase
{
public:
    const char* Name() const override
    {
        return "TexEdit";
    }
    void Init();
    bool ShowGUI() override;
    void ShowContent(FrameBuffer& contentViewFrameBuffer, unsigned int previousFrame, matrix view, matrix projection, const ImVec2 mouseNormalizedPosition) override;

protected:
    
};
