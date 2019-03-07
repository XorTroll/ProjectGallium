
#pragma once
#include <gallium/Platform.hpp>
#include <string>
#include <vector>

extern "C"
{
    #include <types.h>
    #include <utils.h>
    #include <settings.h>
    #include <pki.h>
    #include <nca.h>
    #include <xci.h>
    #include <nax0.h>
    #include <extkeys.h>
    #include <packages.h>
    #include <nso.h>
}

namespace ga
{
    enum class ContentSection
    {
        ExeFs,
        RomFs,
        Logo,
        CNMT,
    };

    enum class ContentType
    {
        Program,
        ContentMeta,
        Control,
        Manual,
        Data,
        DeltaFragment,
    };

    bool IsOnConsole();
    nca_ctx_t ProcessNCA(std::string Path, std::string KeyPath, std::string TitleKey = "");
}