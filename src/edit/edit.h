#pragma once
#include "EditUtils.h"
#include "msys_vector.h"
#include "TextEdit.h"

class Edit
{
public:
    void Init();
    void ShowGUI();
    void ShowContent(matrix view, matrix projection);
    bool Record(bool forceRecording);

protected:
    void ShowContentGUI();

    unsigned int m_tabItem{5};

    TextEdit m_textEdit;

    EditBase* m_editors[1] = {&m_textEdit};
    EditBase* mActiveEditor{nullptr};
    FrameBuffer mContentFrameBuffer;
    int mCurrentFrameBufferIndex{};
    ImVec2 mContentSize{};
    ImVec2 mContentPosition{};
    ImVec2 mContentRenderingSize{};
    ImVec2 mMouseNormalizedPosition{};
};
