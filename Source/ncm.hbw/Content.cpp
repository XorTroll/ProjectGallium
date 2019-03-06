#include <ncm.hbw/Content.hpp>

namespace ncm::hbw
{
    Result Initialize()
    {
        Result rc = ncmInitialize();
        if(rc == 0)
        {
            // Other stuff...?
        }
        return rc;
    }
    void Finalize();
    Content Locate(u64 ApplicationId);
}