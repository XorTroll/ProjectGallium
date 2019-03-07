
#pragma once
#include <stratosphere.hpp>
#include <Content.hpp>

namespace ncm::hbw
{
    // TODO: Try to emulate fsp's IFileSystem commands to make it compatible!
    class IContentFileSystem final : public IServiceObject
    {
        private:
            Result Dummy();
        public:
            DEFINE_SERVICE_DISPATCH_TABLE
            {
                MakeServiceCommandMeta<0, &IContentFileSystem::Dummy>(),
            };
    };
}