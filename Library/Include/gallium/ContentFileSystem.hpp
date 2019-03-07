
#pragma once
#include <gallium/Types.hpp>

namespace ga
{
    class ContentFileSystem
    {
        public:
            ContentFileSystem(nca_ctx_t Context);
            ContentFileSystem(std::string Path, std::string KeyPath, std::string TitleKey = "");
            ~ContentFileSystem();
            ContentType GetType();
            bool IsValid();
            bool HasSection(ContentSection Section);
            bool SetBrowseSection(ContentSection Section);
            ContentSection GetBrowseSection();
            std::vector<std::string> GetFiles();
            std::vector<std::string> GetDirectories();
            std::string GetCwd();
            void NavigateForward(std::string Directory);
            bool NavigateBack();
            std::string FullPathFor(std::string Path);
            u64 GetFileSize(std::string Path);
            u64 GetApplicationId();
            void SaveFile(std::string Path, std::string Out);
        private:
            void InternalSectionFileSave(nca_section_ctx_t *Context, u64 Offset, u64 Size, std::string Out);
            romfs_direntry_t *ResolveCwdRomFs(std::string Cwd);
            nca_section_ctx_t *GetContext(ContentSection Section);
            nca_ctx_t ctx;
            nca_section_ctx_t *exefs;
            nca_section_ctx_t *romfs;
            nca_section_ctx_t *logo;
            nca_section_ctx_t *cnmt;
            ContentSection browse;
            std::string cwd;
    };
}