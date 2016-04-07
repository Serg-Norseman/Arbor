#pragma once
#include "ns\atladd.h"
#include "ui\window\wi.h"
#include <d2d1.h>

ATLADD_BEGIN

template <typename T>
class child_window_impl abstract: public window_impl<T>
{
public:
    explicit child_window_impl(_In_ bool createOnDedicatedThread)
        :
        m_createOnDedicatedThread {createOnDedicatedThread}
    {
    }


protected:
    virtual LRESULT createHandler() override
    {
        LRESULT result = base_class_t::createHandler();
        if (!result)
        {
            HRESULT hr = S_OK;
            if (m_createOnDedicatedThread && !m_animation)
            {
                m_animation = std::make_unique<WAPI animation>();
                hr = m_animation ? S_OK : HRESULT_FROM_WIN32(ERROR_NOT_ENOUGH_MEMORY);
            }
            result = SUCCEEDED(hr) ? 0 : -1;
        }
        return result;
    }

    virtual void destroyHandler() override
    {
        if (m_createOnDedicatedThread)
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

    bool m_createOnDedicatedThread;
};

ATLADD_END
