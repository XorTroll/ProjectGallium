#include <gallium/ContentFileSystem.hpp>

namespace ga
{
    ContentFileSystem::ContentFileSystem(nca_ctx_t Context)
    {
        this->ctx = Context;
        this->cwd = "";
        this->exefs = NULL;
        this->romfs = NULL;
        this->logo = NULL;
        this->cnmt = NULL;
        for(u32 i = 0; i < 4; i++)
        {
            if(this->ctx.section_contexts[i].is_present != 1) continue;
            switch(this->ctx.section_contexts[i].type)
            {
                case ROMFS:
                    this->romfs = &this->ctx.section_contexts[i];
                    break;
                case PFS0:
                    if(this->ctx.section_contexts[i].pfs0_ctx.is_exefs) this->exefs = &this->ctx.section_contexts[i];
                    else
                    {
                        if(i == 2) this->logo = &this->ctx.section_contexts[i];
                        else if(i == 0) this->cnmt = &this->ctx.section_contexts[i];
                    }
                    break;
            }
        }
        if(this->cnmt != NULL) this->browse = ContentSection::CNMT;
        else if(this->exefs != NULL) this->browse = ContentSection::ExeFs;
        else if(this->romfs != NULL) this->browse = ContentSection::RomFs;
    }

    ContentFileSystem::ContentFileSystem(std::string Path, std::string KeyPath, std::string TitleKey) : ContentFileSystem(ProcessNCA(Path, KeyPath, TitleKey))
    {
    }

    ContentFileSystem::~ContentFileSystem()
    {
        this->exefs = NULL;
        this->romfs = NULL;
        this->logo = NULL;
        this->cnmt = NULL;
        nca_free_section_contexts(&this->ctx);
        if(this->ctx.tool_ctx->settings.known_titlekeys.titlekeys != NULL) free(this->ctx.tool_ctx->settings.known_titlekeys.titlekeys);
        if(this->ctx.tool_ctx->file != NULL) fclose(this->ctx.tool_ctx->file);
    }

    ContentType ContentFileSystem::GetType()
    {
        return static_cast<ContentType>(this->ctx.header.content_type);
    }

    bool ContentFileSystem::IsValid()
    {
        return (this->ctx.fixed_sig_validity > 0);
    }

    bool ContentFileSystem::HasSection(ContentSection Section)
    {
        nca_section_ctx_t *sect = this->GetContext(Section);
        return (sect != NULL);
    }

    bool ContentFileSystem::SetBrowseSection(ContentSection Section)
    {
        bool ok = false;
        if(this->HasSection(Section))
        {
            this->browse = Section;
            ok = true;
        }
        return ok;
    }

    std::vector<std::string> ContentFileSystem::GetFiles()
    {
        nca_section_ctx_t *sect = this->GetContext(this->browse);
        std::vector<std::string> fls;
        if(sect == NULL) return fls;
        if(this->browse == ContentSection::RomFs)
        {
            romfs_direntry_t *ecwd = this->ResolveCwdRomFs(this->cwd);
            if(ecwd->file != ROMFS_ENTRY_EMPTY)
            {
                u32 off = ecwd->file;
                while(true)
                {
                    romfs_fentry_t *fent = romfs_get_fentry(sect->romfs_ctx.files, off);
                    if(fent->name_size) fls.push_back(std::string(fent->name, fent->name_size));
                    if(fent->sibling != ROMFS_ENTRY_EMPTY)
                    {
                        off = fent->sibling;
                        continue;
                    }
                    else break;
                }
            }
        }
        else
        {
            for(u32 i = 0; i < sect->pfs0_ctx.header->num_files; i++) fls.push_back(std::string(pfs0_get_file_name(sect->pfs0_ctx.header, i)));
        }
        return fls;
    }

    std::vector<std::string> ContentFileSystem::GetDirectories()
    {
        nca_section_ctx_t *sect = this->GetContext(this->browse);
        std::vector<std::string> dirs;
        if(sect == NULL) return dirs;
        if(this->browse == ContentSection::RomFs)
        {
            romfs_direntry_t *ecwd = this->ResolveCwdRomFs(this->cwd);
            if(ecwd->child != ROMFS_ENTRY_EMPTY)
            {
                u32 off = ecwd->child;
                while(true)
                {
                    romfs_direntry_t *dent = romfs_get_direntry(sect->romfs_ctx.directories, off);
                    if(dent->name_size) dirs.push_back(std::string(dent->name, dent->name_size));
                    if(dent->sibling != ROMFS_ENTRY_EMPTY)
                    {
                        off = dent->sibling;
                        continue;
                    }
                    else break;
                }
            }
        }
        return dirs;
    }

    std::string ContentFileSystem::GetCwd()
    {
        return this->cwd;
    }

    void ContentFileSystem::NavigateForward(std::string Directory)
    {
        std::string res = this->cwd + "/" + Directory;
        this->cwd = res;
    }

    bool ContentFileSystem::NavigateBack()
    {
        if(this->cwd == "") return false;
        std::string parent = this->cwd.substr(0, this->cwd.find_last_of("/"));
        this->cwd = parent;
        return true;
    }

    std::string ContentFileSystem::FullPathFor(std::string Path)
    {
        std::string fpth = Path;
        if(this->browse == ContentSection::RomFs)
        {
            if(this->cwd != "") fpth = this->cwd + "/" + Path;
        }
        return fpth;
    }

    void ContentFileSystem::SaveFile(std::string Path, std::string Out)
    {
        nca_section_ctx_t *sect = this->GetContext(this->browse);
        if(sect == NULL) return;
        u64 foff = 0;
        u64 fsize = 0;
        if(this->browse == ContentSection::RomFs)
        {
            std::string dir = Path.substr(0, Path.find_last_of("/"));
            std::string file = Path;
            if(dir == file) dir = "";
            else file = Path.substr(Path.find_last_of("/") + 1);
            romfs_direntry_t *ecwd = this->ResolveCwdRomFs(dir);
            if(ecwd->file != ROMFS_ENTRY_EMPTY)
            {
                u32 off = ecwd->file;
                while(true)
                {
                    romfs_fentry_t *fent = romfs_get_fentry(sect->romfs_ctx.files, off);
                    if(fent->name_size)
                    {
                        std::string name(fent->name, fent->name_size);
                        if(name == file)
                        {
                            foff = sect->romfs_ctx.romfs_offset + sect->romfs_ctx.header.data_offset + fent->offset;
                            fsize = fent->size;
                            break;
                        }
                    }
                    if(fent->sibling != ROMFS_ENTRY_EMPTY)
                    {
                        off = fent->sibling;
                        continue;
                    }
                    else break;
                }
            }
        }
        else
        {
            for(u32 i = 0; i < sect->pfs0_ctx.header->num_files; i++)
            {
                std::string name(pfs0_get_file_name(sect->pfs0_ctx.header, i));
                if(name == Path)
                {
                    pfs0_file_entry_t *fent = pfs0_get_file_entry(sect->pfs0_ctx.header, i);
                    foff = (sect->pfs0_ctx.superblock->pfs0_offset + pfs0_get_header_size(sect->pfs0_ctx.header) + fent->offset);
                    fsize = fent->size;
                    break;
                }
            }
        }
        if(foff != 0)
        {
            this->InternalSectionFileSave(sect, foff, fsize, Out);
        }
    }

    void ContentFileSystem::InternalSectionFileSave(nca_section_ctx_t *Context, u64 Offset, u64 Size, std::string Out)
    {
        remove(Out.c_str());
        FILE *f_out = os_fopen(Out.c_str(), OS_MODE_WRITE);
        if(f_out == NULL) return;
        uint64_t reads = 0x400000;
        u8 *buf = (u8*)malloc(reads);
        if(buf == NULL) return;
        memset(buf, 0, reads);
        size_t rsize = 0;
        size_t szrem = Size;
        while(szrem)
        {
            nca_section_fseek(Context, Offset);
            rsize = nca_section_fread(Context, buf, std::min(reads, szrem));
            fwrite(buf, 1, rsize, f_out);
            Offset += rsize;
            szrem -= rsize;
        }
        fclose(f_out);
        free(buf);
    }

    romfs_direntry_t *ContentFileSystem::ResolveCwdRomFs(std::string Cwd)
    {
        if(this->romfs == NULL) return NULL;
        romfs_direntry_t *dent;
        std::string path;
        u32 doff = 0;
        while(true)
        {
            dent = romfs_get_direntry(this->romfs->romfs_ctx.directories, doff);
            if(dent)
            {
                if(dent->name_size)
                {
                    std::string pth = path + "/" + std::string(dent->name);
                    if(pth == Cwd) break;
                    else if(Cwd.substr(0, pth.length()) == pth)
                    {
                        path = pth;
                        if(dent->child != ROMFS_ENTRY_EMPTY)
                        {
                            doff = dent->child;
                            continue;
                        }
                        else break;
                    }
                    else if(dent->sibling != ROMFS_ENTRY_EMPTY)
                    {
                        doff = dent->sibling;
                        continue;
                    }
                    else break;
                }
                else if((doff == 0) && (Cwd == "")) break;
                else if(dent->child != ROMFS_ENTRY_EMPTY)
                {
                    doff = dent->child;
                    continue;
                }
                else if(dent->sibling != ROMFS_ENTRY_EMPTY)
                {
                    doff = dent->sibling;
                    continue;
                }
                else break;
            }
        }
        return dent;
    }

    nca_section_ctx_t *ContentFileSystem::GetContext(ContentSection Section)
    {
        nca_section_ctx_t *sect = NULL;
        switch(Section)
        {
            case ContentSection::ExeFs:
                sect = this->exefs;
                break;
            case ContentSection::RomFs:
                sect = this->romfs;
                break;
            case ContentSection::Logo:
                sect = this->logo;
                break;
            case ContentSection::CNMT:
                sect = this->cnmt;
                break;
        }
        return sect;
    }
}