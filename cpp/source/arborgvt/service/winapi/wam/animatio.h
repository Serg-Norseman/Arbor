#pragma once
#include "ns\wapi.h"
#include "service\com\comptr.h"
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

    HRESULT createVariable(_In_ const double fInitialValue, _Outptr_ IUIAnimationVariable2** ppVariable)
    {
        return m_animationManager ? m_animationManager->CreateAnimationVariable(fInitialValue, ppVariable) : E_POINTER;
    }

    HRESULT createStoryboard(_Outptr_ IUIAnimationStoryboard2** ppStoryboard)
    {
        return m_animationManager ? m_animationManager->CreateStoryboard(ppStoryboard) : E_POINTER;
    }

    HRESULT update()
    {
        HRESULT hr = m_animationManager && m_animationTimer ? S_OK : E_POINTER;
        if (SUCCEEDED(hr))
        {
            UI_ANIMATION_SECONDS fCurrentTime;
            hr = m_animationTimer->GetTime(&fCurrentTime);
            if (SUCCEEDED(hr))
            {
                hr = m_animationManager->Update(fCurrentTime);
            }
        }
        return hr;
    }

    HRESULT scheduleStoryboard(_In_ IUIAnimationStoryboard2* pStoryboard)
    {
        HRESULT hr = m_animationTimer ? S_OK : E_POINTER;
        if (SUCCEEDED(hr))
        {
            hr = pStoryboard->Abandon();
            if (SUCCEEDED(hr))
            {
                UI_ANIMATION_SECONDS fCurrentTime;
                hr = m_animationTimer->GetTime(&fCurrentTime);
                if (SUCCEEDED(hr))
                {
                    hr = pStoryboard->Schedule(fCurrentTime);
                }
            }
        }
        return hr;
    }

    HRESULT getStatus(_Out_ UI_ANIMATION_MANAGER_STATUS* pStatus) const
    {
        return m_animationManager ? m_animationManager->GetStatus(pStatus) : E_POINTER;
    }


private:
    ATLADD com_ptr<IUIAnimationManager2> m_animationManager;
    ATLADD com_ptr<IUIAnimationTimer> m_animationTimer;
};

WAPI_END
