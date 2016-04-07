#pragma once
#include "ns\wapi.h"
#include "service\functype.h"
#include <synchapi.h>
#include <atldef.h>
#include <utility>

WAPI_BEGIN

class srw_lock
{
public:
    srw_lock()
    {
        InitializeSRWLock(&m_lock);
    }

    srw_lock(_In_ const srw_lock&) = delete;
    srw_lock& operator =(_In_ const srw_lock&) = delete;
    // Declaration of a move ctor is omitted 'cos (12.8.9.1) and (12.8.9.2) of C++ std.
    // Declaration of a move assignment is omitted 'cos (12.8.20.1) and (12.8.20.3) of C++ std.

    _Acquires_exclusive_lock_(this->m_lock)
    void lockExclusive()
    {
        AcquireSRWLockExclusive(&m_lock);
    }

    _Acquires_shared_lock_(this->m_lock)
    void lockShared()
    {
        AcquireSRWLockShared(&m_lock);
    }

    _Releases_exclusive_lock_(this->m_lock)
    void unlockExclusive() noexcept
    {
        ReleaseSRWLockExclusive(&m_lock);
    }

    _Releases_shared_lock_(this->m_lock)
    void unlockShared() noexcept
    {
        ReleaseSRWLockShared(&m_lock);
    }

    /*
     * When 'tryLockExclusive' and 'tryLockShared' look like that:
     * {
     *     return (0 != TryAcquireSRWLock...(&m_lock));
     * }
     * I end up with warning C26135: "Missing locking annotation ... bla-bla-bla".
     * When the methods look like that:
     * {
     *     return (TryAcquireSRWLockExclusive(&m_lock) ? true : false);
     * }
     * there's no warnings.
     * The second version of code is very silly so I stuck with the first version. Ignore two C26135 warnings -- it's a
     * compiler "feature".
     * VC 2013 RTM (18.00.31101 for x86, VS 2013 Update 4).
     * P.S. VC 2015 Update 1 RTM shows the same C26135 warnings TWICE FOR THE SAME LINE NUMBER in the "Error List" where
     * VC 2015 RTM showed only one warning. But the "Output" window still has one warning per line. That's a bug in
     * Update 1 I believe.
     */
    _When_(0 != return, _Acquires_exclusive_lock_(this->m_lock))
    bool tryLockExclusive()
    {
        return (0 != TryAcquireSRWLockExclusive(&m_lock));
    }

    _When_(0 != return, _Acquires_shared_lock_(this->m_lock))
    bool tryLockShared()
    {
        return (0 != TryAcquireSRWLockShared(&m_lock));
    }


private:
    SRWLOCK m_lock;
};

WAPI_END
