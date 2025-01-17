//
// Copyright 2016 Pixar
//
// Licensed under the Apache License, Version 2.0 (the "Apache License")
// with the following modification; you may not use this file except in
// compliance with the Apache License and the following modification to it:
// Section 6. Trademarks. is deleted and replaced with:
//
// 6. Trademarks. This License does not grant permission to use the trade
//    names, trademarks, service marks, or product names of the Licensor
//    and its affiliates, except as required to comply with Section 4(c) of
//    the License and to reproduce the content of the NOTICE file.
//
// You may obtain a copy of the Apache License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the Apache License with the above modification is
// distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
// KIND, either express or implied. See the Apache License for the specific
// language governing permissions and limitations under the Apache License.
//

#include "pxr/pxr.h"
#include "pxr/base/arch/library.h"
#include "pxr/base/arch/errno.h"

#if defined(ARCH_OS_WINDOWS)
#include <Windows.h>
#else
#include <dlfcn.h>
#endif

PXR_NAMESPACE_OPEN_SCOPE

#if defined(ARCH_OS_WINDOWS)
namespace {
DWORD arch_lastLibraryError = 0;
}
#endif

void* ArchLibraryOpen(const std::string &filename, int flag)
{
#if defined(ARCH_OS_WINDOWS)
    arch_lastLibraryError = 0;
    if (void* result = LoadLibrary(filename.c_str())) {
        return result;
    }
    else {
        arch_lastLibraryError = GetLastError();
        return nullptr;
    }
#elif defined(__EMSCRIPTEN__)
	return nullptr;
#else
    // Clear any unchecked error first.
    (void)dlerror();
    return dlopen(filename.c_str(), flag);
#endif
}

std::string ArchLibraryError()
{
#if defined(ARCH_OS_WINDOWS)
    const DWORD error = arch_lastLibraryError;
    return error ? ArchStrSysError(error) : std::string();
#elif defined(__EMSCRIPTEN__)
	return "Loading plugins dynamically is not supported in EMSCRIPTEN";
#else
    const char* const error = dlerror();
    return error ? std::string(error) : std::string();
#endif
}

int ArchLibraryClose(void* handle)
{
#if defined(ARCH_OS_WINDOWS)
    arch_lastLibraryError = 0;
    // dlclose() returns 0 on success and non-zero on error, the opposite of
    // FreeLibrary().
    int status = ::FreeLibrary(reinterpret_cast<HMODULE>(handle)) ? 0 : -1;
    if (status) {
        arch_lastLibraryError = GetLastError();
    }
#elif defined(__EMSCRIPTEN__)
	int status = 0;
#else
    int status = dlclose(handle);
#endif
    return status;
}

void* ArchLibraryGetSymbolAddress(void* handle, const char* name)
{
#if defined(ARCH_OS_WINDOWS)
    return GetProcAddress(reinterpret_cast<HMODULE>(handle), name);
#elif defined(__EMSCRIPTEN__)
	int status = 0;
#else
    return dlsym(handle, name);
#endif
}

PXR_NAMESPACE_CLOSE_SCOPE
