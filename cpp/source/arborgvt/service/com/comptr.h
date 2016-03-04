#pragma once
#include "ns\atladd.h"
#include <assert.h>
#include <sal.h>
#include <Unknwn.h>
#include <Windows.h>

ATLADD_BEGIN

template <typename T>
class no_add_ref_release: public T
{
private:
    unsigned long __stdcall AddRef() noexcept;
    unsigned long __stdcall Release() noexcept;
};

template <typename T>
class com_ptr
{
public:
    com_ptr() = default;

    com_ptr(_In_ const com_ptr& right) noexcept
        :
        m_p {right.m_p}
    {
        internalAddRef();
    }

    template <typename Q>
    com_ptr(_In_ const com_ptr<Q>& right) noexcept
        :
        m_p {right.m_p}
    {
        internalAddRef();
    }

    template <typename Q>
    com_ptr(_In_ com_ptr<Q>&& right) noexcept
        :
        m_p {right.m_p}
    {
        right.m_p = nullptr;
    }

    com_ptr(_In_ T* p) noexcept
        :
        m_p {p}
    {
        internalAddRef();
    }

    ~com_ptr() noexcept
    {
        internalRelease();
    }

    com_ptr& operator =(_In_ const com_ptr& right) noexcept
    {
        internalCopy(right.m_p);
        return *this;
    }

    template <typename Q>
    com_ptr& operator =(_In_ const com_ptr<Q>& right) noexcept
    {
        internalCopy(right.m_p);
        return *this;
    }

    template <typename Q>
    com_ptr& operator =(_In_ com_ptr<Q>&& right) noexcept
    {
        internalMove(right);
        return *this;
    }

    com_ptr& operator =(_In_ T* p) noexcept
    {
        internalCopy(p);
        return *this;
    }

    no_add_ref_release<T>* operator ->() const noexcept
    {
        return static_cast<no_add_ref_release<T>*> (m_p);
    }

    explicit operator bool() const noexcept
    {
        return nullptr != m_p;
    }

    template <typename Q>
    bool operator ==(_In_ const com_ptr<Q>& right) const noexcept
    {
        return right.m_p == m_p;
    }

    template <typename Q>
    bool operator !=(_In_ const com_ptr<Q>& right) const noexcept
    {
        return right.m_p != m_p;
    }

    void swap(_In_ com_ptr& right)
    {
        T* pTemp = m_p;
        m_p = right.m_p;
        right.m_p = pTemp;
    }

    void reset() noexcept
    {
        internalRelease();
    }

    T* get() const noexcept
    {
        return m_p;
    }

    T* detach() noexcept
    {
        T* pTemp = m_p;
        m_p = nullptr;
        return pTemp;
    }

    void copyTo(T** p) const noexcept
    {
        internalAddRef();
        *p = m_p;
    }

    void attach(_In_ T* p) noexcept
    {
        internalRelease();
        m_p = p;
    }

    T** getAddressOf() noexcept
    {
        assert(nullptr == m_p);
        return &m_p;
    }

    template <typename Q>
    com_ptr<Q> as() const noexcept
    {
        com_ptr<Q> temp {};
        return SUCCEEDED(m_p->QueryInterface(temp.getAddressOf())) ? temp : nullptr;
    }

    HRESULT coCreateInstance(_In_ const IID& iid, _Inout_opt_ IUnknown* pOuter, _In_ DWORD nClsContext) noexcept
    {
        assert(nullptr == m_p);
        return ::CoCreateInstance(iid, pOuter, nClsContext, __uuidof(T), reinterpret_cast<void**> (&m_p));
    }


private:
    void internalAddRef() const noexcept
    {
        if (m_p)
        {
            m_p->AddRef();
        }
    }

    void internalRelease() noexcept
    {
        T* pTemp = m_p;
        if (pTemp)
        {
            m_p = nullptr;
            pTemp->Release();
        }
    }

    void internalCopy(_In_ T* p) noexcept
    {
        if (m_p != p)
        {
            internalRelease();
            m_p = p;
            internalAddRef();
        }
    }

    template <typename Q>
    void internalMove(_In_ com_ptr<Q>& right) noexcept
    {
        if (m_p != right.m_p)
        {
            internalRelease();
            m_p = right.m_p;
            right.m_p = nullptr;
        }
    }

    T* m_p = nullptr;
};

ATLADD_END

template <typename T>
inline void swap(_In_ ATLADD com_ptr<T>& left, _In_ ATLADD com_ptr<T>& right) noexcept
{
    left.swap(right);
}
