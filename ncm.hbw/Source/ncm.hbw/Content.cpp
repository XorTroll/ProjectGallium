#include <ncm.hbw/Content.hpp>
#include <cstdlib>
#include <cstring>

namespace ncm::hbw
{
    std::string ContentId::ToString()
    {
        char idstr[FS_MAX_PATH] = { 0 };
        u64 lower = __bswap64(*(u64*)this->Id.c);
        u64 upper = __bswap64(*(u64*)(this->Id.c + 8));
        snprintf(idstr, FS_MAX_PATH, "%016lx%016lx", lower, upper);
        return std::string(idstr);
    }

    Result Initialize()
    {
        Result rc = ncmInitialize();
        if(rc == 0)
        {
            FsFileSystem nsys;
            rc = fsOpenBisFileSystem(&nsys, 31, "");
            if(rc == 0) fsdevMountDevice("nsys", nsys);
            FsFileSystem nuser;
            rc = fsOpenBisFileSystem(&nuser, 30, "");
            if(rc == 0) fsdevMountDevice("nuser", nuser);
        }
        return rc;
    }

    void Finalize()
    {
        fsdevUnmountDevice("nsys");
        fsdevUnmountDevice("nuser");
        ncmExit();
    }

    NcmMetaRecord GetMetaRecordFrom(NcmContentMetaDatabase *metadb, u64 ApplicationId)
    {
        NcmMetaRecord mrc;
        memset(&mrc, 0, sizeof(NcmMetaRecord));
        size_t recsz = sizeof(NcmMetaRecord) * 256;
        NcmMetaRecord *mrcs = (NcmMetaRecord*)malloc(recsz);
        u32 total = 0;
        u32 wrt = 0;
        Result rc = ncmContentMetaDatabaseList(metadb, 0, 0, 0, UINT64_MAX, mrcs, recsz, &wrt, &total);
        if(rc == 0) if(wrt > 0) for(u32 i = 0; i < wrt; i++) if(mrcs[i].titleId == ApplicationId)
        {
            memcpy(&mrc, &mrcs[i], sizeof(NcmMetaRecord));
            break;
        }
        return mrc;
    }

    Content Locate(u64 ApplicationId)
    {
        Content cnt;
        memset(&cnt, 0, sizeof(Content));
        cnt.Valid = false;
        cnt.ApplicationId = ApplicationId;
        FsStorageId stid = FsStorageId_None; // aka 0, not found
        NcmContentMetaDatabase cmdb;
        Result rc = 0;
        NcmMetaRecord mrc;
        for(u32 i = 2; i <= 5; i++) // Iterate over NAND/SD/Cart storage Ids
        {
            rc = ncmOpenContentMetaDatabase((FsStorageId)i, &cmdb);
            if(rc == 0)
            {
                mrc = GetMetaRecordFrom(&cmdb, ApplicationId);
                // GetMetaRecordFrom's returned record will be all zeroes if not found
                // As simple as check the app Id to see whether it's valid
                if(mrc.titleId != 0)
                {
                    stid = (FsStorageId)i;
                    // Not close the database, we'll need it later
                    break;
                }
            }
            serviceClose(&cmdb.s);
        }
        if(stid != FsStorageId_None) // Else we return with cnt.Valid set to false
        {
            cnt.Record = mrc;
            cnt.Storage = stid;
            // Time to get all the NCAs with the database opened before
            if(rc == 0)
            {
                for(u32 i = 0; i < 6; i++) // Loop for every NCA type
                {
                    NcmNcaId out;
                    rc = ncmContentMetaDatabaseGetContentIdByType(&cmdb, (NcmContentType)i, &mrc, &out);
                    if(rc == 0) switch(i)
                    {
                        case 0: // CNMT NCA
                            cnt.ContentMetaId = { true, out };
                            break;
                        case 1: // Program NCA
                            cnt.ProgramId = { true, out };
                            break;
                        case 2: // Data NCA
                            cnt.DataId = { true, out };
                            break;
                        case 3: // Control NCA
                            cnt.ControlId = { true, out };
                            break;
                        case 4: // Html docs NCA
                            cnt.HtmlDocumentsId = { true, out };
                            break;
                        case 5: // Legal info NCA
                            cnt.LegalInfoId = { true, out };
                            break;
                    }
                }
            }
            serviceClose(&cmdb.s);
            cnt.Valid = true;
            // Set valid to true at the very end, to ensure that at least any NCA was got
        }
        return cnt;
    }
}