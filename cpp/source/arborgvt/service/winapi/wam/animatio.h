#pragma once
#include "ns/wapi.h"
#include "service/com/comptr.h"
#include <dcomp.h>
#include <UIAnimation.h>

WAPI_BEGIN

class animation
{
public:
    animation()
        :
        m_animationManager {},
        m_animationTimer {}
    {
        if (SUCCEEDED(m_animationManager.coCreateInstance(CLSID_UIAnimationManager2, nullptr, CLSCTX_INPROC_SERVER)))
        {
            if (FAILED(m_animationTimer.coCreateInstance(CLSID_UIAnimationTimer, nullptr, CLSCTX_INPROC_SERVER)))
            {
                m_animationManager.reset();
            }
        }
    }

    HRESULT createVariable(_In_ const double initialValue, _Outptr_ IUIAnimationVariable2** variable)
    {
        return m_animationManager ? m_animationManager->CreateAnimationVariable(initialValue, variable) : E_POINTER;
    }

    HRESULT createStoryboard(_Outptr_ IUIAnimationStoryboard2** storyboard)
    {
        return m_animationManager ? m_animationManager->CreateStoryboard(storyboard) : E_POINTER;
    }

    HRESULT update()
    {
        HRESULT hr = m_animationManager && m_animationTimer ? S_OK : E_POINTER;
        if (SUCCEEDED(hr))
        {
            UI_ANIMATION_SECONDS currentTime;
            hr = m_animationTimer->GetTime(&currentTime);
            if (SUCCEEDED(hr))
            {
                hr = m_animationManager->Update(currentTime);
            }
        }
        return hr;
    }

    HRESULT scheduleStoryboard(_In_ IUIAnimationStoryboard2* storyboard)
    {
        HRESULT hr = m_animationTimer ? S_OK : E_POINTER;
        if (SUCCEEDED(hr))
        {
            hr = storyboard->Abandon();
            if (SUCCEEDED(hr))
            {
                UI_ANIMATION_SECONDS currentTime;
                hr = m_animationTimer->GetTime(&currentTime);
                if (SUCCEEDED(hr))
                {
                    hr = storyboard->Schedule(currentTime);
                }
            }
        }
        return hr;
    }

    HRESULT getStatus(_Out_ UI_ANIMATION_MANAGER_STATUS* status) const
    {
        return m_animationManager ? m_animationManager->GetStatus(status) : E_POINTER;
    }


private:
    ATLADD com_ptr<IUIAnimationManager2> m_animationManager;
    ATLADD com_ptr<IUIAnimationTimer> m_animationTimer;
};

WAPI_END
