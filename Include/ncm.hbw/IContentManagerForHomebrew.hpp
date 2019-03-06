
#pragma once
#include <ncm.hbw/IContentFileSystem.hpp>

namespace ncm::hbw
{
    class IContentManagerForHomebrew final : public IServiceObject
    {
        private:
            Result Initialize();
            Result HasInitialized(Out<bool> Initialized);
            Result MountContentByType(u64 ApplicationId, NcmContentType NCAType, u8 Section, Out<std::shared_ptr<IContentFileSystem>> OutVFS);
            // Result Pause();
            /*
            Result CountQueue(Out<u32> Count);
            Result ClearQueue();
            Result EraseFromQueueByIndex(u32 Index);
            Result GetFromQueueByIndex(u32 Index, OutBuffer<QueueInput> Input);
            Result IsPlaying(Out<bool> Playing);
            Result MoveForward();
            Result MoveForwardAndPop();
            Result MoveBackward();
            */
        public:
            DEFINE_SERVICE_DISPATCH_TABLE
            {
                MakeServiceCommandMeta<0, &ITestService::Initialize>(),
                MakeServiceCommandMeta<1, &ITestService::Play>(),
                MakeServiceCommandMeta<2, &ITestService::AddToQueue>(),
            };
        private:
            AudioPlayer *audpl;
            std::vector<QueueInput*> queue;
    };

    static const u32 Module = 360;
    static const Result ResultNotStarted = MAKERESULT(Module, 1);
    static const Result ResultAlreadyStarted = MAKERESULT(Module, 2);
    static const Result ResultEmptyQueue = MAKERESULT(Module, 3);
    static const Result ResultAlreadyAdded = MAKERESULT(Module, 4);
    static const Result ResultInvalidAudio = MAKERESULT(Module, 5);
    static const Result ResultInvalidQueueIndex = MAKERESULT(Module, 6);
}