
#pragma once
#include <gallium.hpp>
#include <switch.h>
#include <string>
#include <vector>

namespace ncm::hbw
{
    struct ContentId
    {
        bool Valid;
        NcmNcaId Id;

        std::string ToString();
    };

    struct Content
    {
        bool Valid;
        u64 ApplicationId;
        FsStorageId Storage;
        NcmMetaRecord Record;
        ContentId ContentMetaId;
        ContentId ProgramId;
        ContentId DataId;
        ContentId ControlId;
        ContentId HtmlDocumentsId;
        ContentId LegalInfoId;
    };

    Result Initialize();
    void Finalize();
    NcmMetaRecord GetMetaRecordFrom(NcmContentMetaDatabase *metadb, u64 ApplicationId);
    Content Locate(u64 ApplicationId);
}