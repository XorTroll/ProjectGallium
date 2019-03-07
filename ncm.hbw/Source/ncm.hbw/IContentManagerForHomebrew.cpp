#include <ncm.hbw/IContentManagerForHomebrew.hpp>
#include <fstream>

namespace ncm::hbw
{
    Result IContentManagerForHomebrew::MountContentByType(u64 ApplicationId, NcmContentType Type, u8 Section, Out<std::shared_ptr<IContentFileSystem>> OutVFS)
    {
        Result rc = 0;
        Content cnt = Locate(ApplicationId);
        if(cnt.Valid)
        {
            ContentId base;
            switch(Type)
            {
                case NcmContentType_CNMT:
                    base = cnt.ContentMetaId;
                    break;
                case NcmContentType_Program:
                    base = cnt.ProgramId;
                    break;
                case NcmContentType_Data:
                    base = cnt.DataId;
                    break;
                case NcmContentType_Icon:
                    base = cnt.ControlId;
                    break;
                case NcmContentType_Doc:
                    base = cnt.HtmlDocumentsId;
                    break;
                case NcmContentType_Info:
                    base = cnt.LegalInfoId;
                    break;
            }
            if(base.Valid)
            {
                std::string pbase = "sdmc:/"; // Default to SD card
                if(cnt.Storage == FsStorageId_NandSystem) pbase = "nsys:/";
                else if(cnt.Storage == FsStorageId_NandUser) pbase = "nuser:/";
                std::string cntpath = pbase + "/Contents/registered/" + base.ToString() + ".nca";
                remove("sdmc:/ncm.debug.txt");
                std::ofstream ofs("sdmc:/ncm.debug.txt");
                ofs << cntpath;
                ofs.close();
            }
            else rc = ResultRequiredContentIdNotPresent;
        }
        else rc = ResultContentNotFound;
        return rc;
    }
}