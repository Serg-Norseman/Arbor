#pragma once
#include "ns\atladd.h"
#include "ui\window\wi.h"
#include <d2d1.h>

ATLADD_BEGIN

template <typename T>
class child_window_impl abstract: public window_impl<T>
{
public:
    explicit child_window_impl(_In_ bool bCreateOnDedicatedThread)
        :
        m_bCreateOnDedicatedThread {bCreateOnDedicatedThread}
    {
    }


protected:
    virtual LRESULT createHandler() override
    {
        LRESULT nResult = base_class_t::createHandler();
        if (!nResult)
        {
            HRESULT hr = S_OK;
            if (m_bCreateOnDedicatedThread && !m_animation)
            {
                m_animation = std::make_unique<WAPI animation>();
                hr = m_animation ? S_OK : HRESULT_FROM_WIN32(ERROR_NOT_ENOUGH_MEMORY);
            }
            nResult = SUCCEEDED(hr) ? 0 : -1;
        }
        return nResult;
    }

    virtual void destroyHandler() override
    {
        if (m_bCreateOnDedicatedThread)
        {
            PostQuitMessage(0);
            /*
             * Since the code is part of the DLL we need to release `static` members manually.
             */
            releaseDevice();
            releaseAllFactories();
        }
    }


private:
    typedef window_impl<T> base_class_t;

    bool m_bCreateOnDedicatedThread;
};

ATLADD_END
