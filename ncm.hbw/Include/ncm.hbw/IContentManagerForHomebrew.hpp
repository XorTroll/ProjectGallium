
#pragma once
#include <ncm.hbw/IContentFileSystem.hpp>

namespace ncm::hbw
{
    class IContentManagerForHomebrew final : public IServiceObject
    {
        private:
            Result MountContentByType(u64 ApplicationId, NcmContentType Type, u8 Section, Out<std::shared_ptr<IContentFileSystem>> OutVFS);
        public:
            DEFINE_SERVICE_DISPATCH_TABLE
            {
                MakeServiceCommandMeta<0, &IContentManagerForHomebrew::MountContentByType>(),
            };
    };

    static const u32 Module = 362;
    static const Result ResultContentNotFound = MAKERESULT(Module, 1);
    static const Result ResultRequiredContentIdNotPresent = MAKERESULT(Module, 2);
    static const Result ResultFailedToMountNAND = MAKERESULT(Module, 3);
}