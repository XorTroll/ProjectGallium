
// ncm:hbw custom sysmodule / service
// More simple system for NCA mounting, including key derivation (thanks shchmue / Lockpick!)

#include <ncm.hbw.hpp>

extern "C"
{
    extern u32 __start__;
    u32 __nx_applet_type = AppletType_None;
    #define INNER_HEAP_SIZE 0x20000
    size_t nx_inner_heap_size = INNER_HEAP_SIZE;
    char nx_inner_heap[INNER_HEAP_SIZE];
    void __libnx_initheap(void);
    void __appInit(void);
    void __appExit(void);
}


void __libnx_initheap(void)
{
	void* addr = nx_inner_heap;
	size_t size = nx_inner_heap_size;
	extern char* fake_heap_start;
	extern char* fake_heap_end;
	fake_heap_start = (char*)addr;
	fake_heap_end = (char*)addr + size;
}

void __appInit(void)
{
    svcSleepThread(8000000000L);
    Result rc = smInitialize();
    if(rc != 0) exit(0);
    rc = fsInitialize();
    if(rc != 0) exit(0);
    rc = fsdevMountSdmc();
    if(rc != 0) exit(0);
    rc = ncm::hbw::Initialize();
    if(rc != 0) exit(0);
}

void __appExit(void)
{
    ncm::hbw::Finalize();
    fsdevUnmountAll();
    fsExit();
    smExit();
}

int main(int argc, char **argv)
{
    auto server = new WaitableManager(1);
    server->AddWaitable(new ServiceServer<ncm::hbw::IContentManagerForHomebrew>("ncm:hbw", 10));
    server->Process();
    delete server;
    return 0;
}