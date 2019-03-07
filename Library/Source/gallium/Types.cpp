#include <gallium/Types.hpp>
#include <unistd.h>
#include <cstdio>

namespace ga
{
    bool IsOnConsole()
    {
        return (bool)GA_ONCONSOLE;
    }

    nca_ctx_t ProcessNCA(std::string Path, std::string KeyPath, std::string TitleKey)
    {
        int outfd = dup(STDOUT_FILENO);
        int errfd = dup(STDERR_FILENO);
        freopen("/tmpGallium_out.log", "w", stdout);
        freopen("/tmpGallium_err.log", "w", stderr);
        hactool_ctx_t tool_ctx;
        hactool_ctx_t base_ctx;
        nca_ctx_t nca_ctx;
        filepath_t keypath;
        nca_init(&nca_ctx);
        memset(&tool_ctx, 0, sizeof(tool_ctx));
        memset(&base_ctx, 0, sizeof(base_ctx));
        filepath_init(&keypath);
        nca_ctx.tool_ctx = &tool_ctx;
        nca_ctx.is_cli_target = true;
        nca_ctx.tool_ctx->file_type = FILETYPE_NCA;
        base_ctx.file_type = FILETYPE_NCA; 
        nca_ctx.tool_ctx->action = ACTION_INFO | ACTION_EXTRACT;
        pki_initialize_keyset(&tool_ctx.settings.keyset, KEYSET_RETAIL);
        if(TitleKey != "")
        {
            parse_hex_key(nca_ctx.tool_ctx->settings.cli_titlekey, TitleKey.c_str(), 16);
            nca_ctx.tool_ctx->settings.has_cli_titlekey = 1;
        }
        filepath_set(&keypath, KeyPath.c_str());
        FILE *keyfile = NULL;
        if(keypath.valid == VALIDITY_VALID) keyfile = os_fopen(keypath.os_path, OS_MODE_READ);
        if(keyfile != NULL)
        {
            extkeys_initialize_keyset(&tool_ctx.settings.keyset, keyfile);
            pki_derive_keys(&tool_ctx.settings.keyset);
            fclose(keyfile);
        }
        if((tool_ctx.file = fopen(Path.c_str(), "rb")) == NULL && tool_ctx.file_type != FILETYPE_BOOT0) return nca_ctx;
        if(nca_ctx.tool_ctx->base_nca_ctx != NULL)
        {
            memcpy(&base_ctx.settings.keyset, &tool_ctx.settings.keyset, sizeof(nca_keyset_t));
            base_ctx.settings.known_titlekeys = tool_ctx.settings.known_titlekeys;
            nca_ctx.tool_ctx->base_nca_ctx->tool_ctx = &base_ctx;
            nca_process(nca_ctx.tool_ctx->base_nca_ctx);
            int found_romfs = 0;
            for(u32 i = 0; i < 4; i++)
            {
                if(nca_ctx.tool_ctx->base_nca_ctx->section_contexts[i].is_present && nca_ctx.tool_ctx->base_nca_ctx->section_contexts[i].type == ROMFS)
                {
                    found_romfs = 1;
                    break;
                }
            }
            if(found_romfs == 0) return nca_ctx;
        }
        nca_ctx.file = tool_ctx.file;
        nca_process(&nca_ctx);
        fclose(stdout);
        fclose(stderr);
        dup2(outfd, STDOUT_FILENO);
        dup2(errfd, STDERR_FILENO);
        stdout = fdopen(STDOUT_FILENO, "w");
        stderr = fdopen(STDERR_FILENO, "w");
        close(outfd);
        close(errfd);
        remove("/tmpGallium_out.log");
        remove("/tmpGallium_err.log");
        return nca_ctx;
    }
}