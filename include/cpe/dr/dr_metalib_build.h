#ifndef CPE_DR_METALIB_BUILD_H
#define CPE_DR_METALIB_BUILD_H
#include "cpe/utils/error.h"
#include "cpe/utils/buffer.h"
#include "cpe/dr/dr_types.h"
#include "cpe/dr/dr_external.h"

#ifdef __cplusplus
extern "C" {
#endif

struct DRInBuildMetaLib * dr_inbuild_create_lib(void);
void dr_inbuild_free_lib(struct DRInBuildMetaLib * ctx);

int dr_inbuild_build_lib(
    mem_buffer_t buffer,
    struct DRInBuildMetaLib * inBuildMetaLib,
    error_monitor_t er);

/*detail operations*/
struct DRInBuildMacro * dr_inbuild_metalib_add_macro(struct DRInBuildMetaLib * inBuildMetaLib);
void dr_inbuild_metalib_remove_macro(struct DRInBuildMetaLib * inBuildMetaLib, struct DRInBuildMacro * macro);
int dr_inbuild_metalib_add_macro_to_index(struct DRInBuildMetaLib * inBuildMetaLib, struct DRInBuildMacro * macro);
struct DRInBuildMacro * dr_inbuild_metalib_find_macro(struct DRInBuildMetaLib * inBuildMetaLib, const char * macro_name);

struct DRInBuildMeta * dr_inbuild_metalib_add_meta(struct DRInBuildMetaLib * inBuildMetaLib);
void dr_inbuild_metalib_remove_meta(struct DRInBuildMetaLib * inBuildMetaLib, struct DRInBuildMeta * meta);

int dr_inbuild_meta_current_version(struct DRInBuildMeta * meta);
void dr_inbuild_meta_set_type(struct DRInBuildMeta * meta, int type);
void dr_inbuild_meta_set_align(struct DRInBuildMeta * meta, int align);
void dr_inbuild_meta_set_id(struct DRInBuildMeta * meta, int id);
void dr_inbuild_meta_set_base_version(struct DRInBuildMeta * meta, int version);
void dr_inbuild_meta_set_current_version(struct DRInBuildMeta * meta, int version);
void dr_inbuild_meta_set_name(struct DRInBuildMeta * meta, const char * name);
void dr_inbuild_meta_set_desc(struct DRInBuildMeta * meta, const char * desc);

struct DRInBuildMetaEntry * dr_inbuild_meta_add_entry(struct DRInBuildMeta * meta);
void dr_inbuild_meta_remove_entry(struct DRInBuildMeta * meta, struct DRInBuildMetaEntry * entry);

int dr_inbuild_entry_version(struct DRInBuildMetaEntry * entry);
void dr_inbuild_entry_set_type(struct DRInBuildMetaEntry * entry, const char * type_name);
void dr_inbuild_entry_set_id(struct DRInBuildMetaEntry * entry, int id);
void dr_inbuild_entry_set_array_count(struct DRInBuildMetaEntry * entry, int array_count);
void dr_inbuild_entry_set_version(struct DRInBuildMetaEntry * entry, int version);
void dr_inbuild_entry_set_name(struct DRInBuildMetaEntry * entry, const char * name);
void dr_inbuild_entry_set_desc(struct DRInBuildMetaEntry * entry, const char * desc);

#ifdef __cplusplus
}
#endif


#endif
