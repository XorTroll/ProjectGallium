#include <switch.h>
#include <vector>

namespace ncm::hbw
{
    struct Content
    {
        u64 ApplicationId;
        FsStorageId Storage;
        NcmMetaRecord Record;
        NcmNcaId ContentMetaId;
        NcmNcaId ProgramId;
        NcmNcaId ControlId;
        // TODO: implement more NCA types
    };

    Result Initialize();
    void Finalize();
    Content Locate(u64 ApplicationId);
}