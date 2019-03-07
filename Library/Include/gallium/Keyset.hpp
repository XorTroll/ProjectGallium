
#pragma once
#include <gallium/Types.hpp>

namespace ga
{
    struct ScrappedKeys
    {
        std::vector<u8> HeaderKey;
        std::vector<u8> KeyAreaKeyApplicationSource;
    };

    struct Keyset
    {
        bool Derived;
        union
        {
            std::string Path;
            ScrappedKeys DerivedKeys;
        };
    };
}