#pragma once
#include "sdkver.h"
#include <rpc.h>
#include <rpcndr.h>
#include <Unknwn.h>
#include <Windows.h>

/**
 * `IArborVisual` interface.
 * `IArborVisual` interface represents a window object where graph representation is rendered.
 *
 * The `IArborVisual` interface has these methods.
 * `createWindow` creates window USER object.
 */
MIDL_INTERFACE("5923B678-E139-4334-A138-E0EA2298AA08")
IArborVisual: public IUnknown
{
public:
    virtual HRESULT STDMETHODCALLTYPE createWindow(_Out_ HWND* handle) = 0;
};
