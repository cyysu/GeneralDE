#include <assert.h>
#include "libxml/tree.h"
#include "libxml/xmlstring.h"
#include "cpe/pal/pal_string.h"
#include "cpe/pal/pal_strings.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "cpe/utils/stream_buffer.h"
#include "ui/model/ui_data_layout.h"
#include "ui/model/ui_data_src.h"
#include "ui/model/ui_data_mgr.h"
#include "ui_np_utils.h"
#include "ui_np_load_utils.h"

struct ui_np_load_layout_ctx {
    ui_data_mgr_t m_mgr;
    ui_data_src_t m_src;
    error_monitor_t m_em;
    ui_data_layout_t m_layout;
    ui_data_control_t m_cur_control;
    UI_CONTROL_BASIC * m_data_basic;
    UI_CONTROL_TEMPLATE * m_data_template;
    UI_CONTROL_LAYOUT * m_data_layout;
    UI_CONTROL_STATE * m_data_state;
    UI_CONTROL_ANIM * m_data_anim;
    UI_CONTROL_SOUND * m_data_sound;
    UI_CONTROL_WINDOW * m_data_window;
    UI_CONTROL_ED * m_data_ed;
    char m_cur_tag_name[64];
};

static void ui_np_load_control_init_data(struct ui_np_load_layout_ctx * ctx);
static void ui_np_load_control_common_attrs(struct ui_np_load_layout_ctx * ctx, int nb_attributes, const xmlChar** attributes);

static void ui_np_load_layout_startElement(
        void* inputCtx,
        const xmlChar* localname,
        const xmlChar* prefix,
        const xmlChar* URI,
        int nb_namespaces,
        const xmlChar** namespaces,
        int nb_attributes,
        int nb_defaulted,
        const xmlChar** attributes)
{
    struct ui_np_load_layout_ctx * ctx = (struct ui_np_load_layout_ctx *)(inputCtx);
    uint8_t i;

    for(i = UI_DATA_CONTROL_TYPE_MIN; i < UI_DATA_CONTROL_TYPE_MAX; ++i) {
        if (strcmp((const char *)localname, ui_data_np_control_tag_name(i)) == 0) {
            ctx->m_cur_control = ui_data_control_create(ctx->m_layout, ctx->m_cur_control, i);
            if (ctx->m_cur_control  == NULL) {
                CPE_ERROR(ctx->m_em, "create actor fail!");
                ui_np_load_control_init_data(ctx);
                return;
            }

            ui_np_load_control_init_data(ctx);
            ui_np_load_control_common_attrs(ctx, nb_attributes, attributes);
            return;
        }
    }

    if (strcmp((const char *)localname, "RenderPT") == 0) {
        if (ctx->m_data_layout) {
            UI_NP_XML_READ_ATTR_UNIT_VECTOR_2(&ctx->m_data_layout->render_pt);
        }
    }
    else if (strcmp((const char *)localname, "EditorPT") == 0) {
        if (ctx->m_data_ed) {
            UI_NP_XML_READ_ATTR_VECTOR_2(&ctx->m_data_ed->editor_pt);
        }
    }
    else if (strcmp((const char *)localname, "EditorSZ") == 0) {
        if (ctx->m_data_ed) {
            UI_NP_XML_READ_ATTR_VECTOR_2(&ctx->m_data_ed->editor_sz);
        }
    }
    else if (strcmp((const char *)localname, "EditorSZ") == 0) {
        if (ctx->m_data_ed) {
            UI_NP_XML_READ_ATTR_VECTOR_2(&ctx->m_data_ed->editor_sz);
        }
    }
    else if (strcmp((const char *)localname, "EditorPD") == 0) {
        if (ctx->m_data_ed) {
            UI_NP_XML_READ_ATTR_RECT(&ctx->m_data_ed->editor_pd);
        }
    }
    else if (strcmp((const char *)localname, "RenderSZ") == 0) {
        if (ctx->m_data_layout) {
            UI_NP_XML_READ_ATTR_UNIT_VECTOR_2(&ctx->m_data_layout->render_sz);
        }
    }
    else if (strcmp((const char *)localname, "ClientPD") == 0) {
        if (ctx->m_data_layout) {
            UI_NP_XML_READ_ATTR_UNIT_RECT(&ctx->m_data_layout->client_pd);
        }
    }
    else if (strcmp((const char *)localname, "Pivot") == 0) {
        if (ctx->m_data_layout) {
            UI_NP_XML_READ_ATTR_VECTOR_2(&ctx->m_data_layout->pivot);
        }
    }
    else if (strcmp((const char *)localname, "Scale") == 0) {
        if (ctx->m_data_layout) {
            UI_NP_XML_READ_ATTR_VECTOR_2(&ctx->m_data_layout->scale);
        }
    }
    else if (strcmp((const char *)localname, "Angle") == 0) {
        if (ctx->m_data_layout) {
            UI_NP_XML_READ_ATTR_VECTOR_3(&ctx->m_data_layout->angle);
        }
    }
    else if (strcmp((const char *)localname, "Color") == 0) {
        if (ctx->m_data_layout) {
            UI_NP_XML_READ_ATTR_COLOR(&ctx->m_data_layout->color);
        }
    }
    else if (strcmp((const char *)localname, "GrayColor") == 0) {
        if (ctx->m_data_layout) {
            UI_NP_XML_READ_ATTR_COLOR(&ctx->m_data_layout->gray_color);
        }
    }
    else {
        strncpy(ctx->m_cur_tag_name, (const char*)localname, sizeof(ctx->m_cur_tag_name));
    }
}

static void ui_np_load_layout_endElement(
        void* inputCtx,
        const xmlChar* localname,
        const xmlChar* prefix,
        const xmlChar* URI)
{
    uint8_t i;
    struct ui_np_load_layout_ctx * ctx = (struct ui_np_load_layout_ctx *)(inputCtx);
    ctx->m_cur_tag_name[0] = 0;

    if (ctx->m_cur_control == NULL) return;

    for(i = UI_DATA_CONTROL_TYPE_MIN; i < UI_DATA_CONTROL_TYPE_MAX; ++i) {
        if (strcmp((const char *)localname, ui_data_np_control_tag_name(i)) == 0) {
            ctx->m_cur_control = ui_data_control_parent(ctx->m_cur_control);
            ui_np_load_control_init_data(ctx);
            return;
        }
    }
}

static void ui_np_load_layout_characters(void * inputCtx, const xmlChar *ch, int len) {
    struct ui_np_load_layout_ctx * ctx = (struct ui_np_load_layout_ctx *)(inputCtx);

    if (strcmp(ctx->m_cur_tag_name, "TemplateLink") == 0) {
        if (ctx->m_data_template) {
            UI_NP_XML_READ_VALUE_BOOL(ctx->m_data_template->is_link);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "TemplateLinkCtrl") == 0) {
        if (ctx->m_data_template) {
            UI_NP_XML_READ_VALUE_STRING(ctx->m_data_template->link_control);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "UserText") == 0) {
        if (ctx->m_data_basic) {
            UI_NP_XML_READ_VALUE_STRING(ctx->m_data_basic->user_text);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "AlignHorz") == 0) {
        if (ctx->m_data_layout) {
            UI_NP_XML_READ_VALUE_INT(uint8_t, ctx->m_data_layout->align_horz);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "AlignVert") == 0) {
        if (ctx->m_data_layout) {
            UI_NP_XML_READ_VALUE_INT(uint8_t, ctx->m_data_layout->align_vert);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "Visible") == 0) {
        if (ctx->m_data_state) {
            UI_NP_XML_READ_VALUE_BOOL(ctx->m_data_state->visiable);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "Alpha") == 0) {
        if (ctx->m_data_layout) {
            UI_NP_XML_READ_VALUE_FLOAT(ctx->m_data_layout->alpha);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "DrawAlign") == 0) {
        if (ctx->m_data_layout->draw_align) {
            UI_NP_XML_READ_VALUE_INT(uint8_t, ctx->m_data_layout->draw_align);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "DrawColor") == 0) {
        if (ctx->m_data_layout) {
            UI_NP_XML_READ_VALUE_BOOL(ctx->m_data_layout->draw_color);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "DrawFrame") == 0) {
        if (ctx->m_data_layout) {
            UI_NP_XML_READ_VALUE_BOOL(ctx->m_data_layout->draw_frame);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "DrawInner") == 0) {
        if (ctx->m_data_layout) {
            UI_NP_XML_READ_VALUE_BOOL(ctx->m_data_layout->draw_inner);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "Enable") == 0) {
        if (ctx->m_data_state) {
            UI_NP_XML_READ_VALUE_BOOL(ctx->m_data_state->enable);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "AlwaysTop") == 0) {
        if (ctx->m_data_state) {
            UI_NP_XML_READ_VALUE_BOOL(ctx->m_data_state->always_top);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "AcceptPTLS") == 0) {
        if (ctx->m_data_state) {
            UI_NP_XML_READ_VALUE_BOOL(ctx->m_data_state->accept_pt_ls);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "AcceptSZLS") == 0) {
        if (ctx->m_data_state) {
            UI_NP_XML_READ_VALUE_BOOL(ctx->m_data_state->accept_sz_ls);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "AcceptHits") == 0) {
        if (ctx->m_data_state) {
            UI_NP_XML_READ_VALUE_BOOL(ctx->m_data_state->accept_hits);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "AcceptClip") == 0) {
        if (ctx->m_data_state) {
            UI_NP_XML_READ_VALUE_BOOL(ctx->m_data_state->accept_clip);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "ParentClip") == 0) {
        if (ctx->m_data_state) {
            UI_NP_XML_READ_VALUE_BOOL(ctx->m_data_state->parent_clip);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "AcceptMove") == 0) {
        if (ctx->m_data_state) {
            UI_NP_XML_READ_VALUE_BOOL(ctx->m_data_state->accept_move);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "FireAnimClip") == 0) {
        if (ctx->m_data_anim) {
            UI_NP_XML_READ_VALUE_BOOL(ctx->m_data_anim->fire_anim_clip);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "ShowSFXFile") == 0) {
        if (ctx->m_data_sound) {
            UI_NP_XML_READ_VALUE_STRING(ctx->m_data_sound->show_sfx_file);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "HideSFXFile") == 0) {
        if (ctx->m_data_sound) {
            UI_NP_XML_READ_VALUE_STRING(ctx->m_data_sound->hide_sfx_file);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "DnSFXFile") == 0) {
        if (ctx->m_data_sound) {
            UI_NP_XML_READ_VALUE_STRING(ctx->m_data_sound->down_sfx_file);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "UpSFXFile") == 0) {
        if (ctx->m_data_sound) {
            UI_NP_XML_READ_VALUE_STRING(ctx->m_data_sound->rise_sfx_file);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "PushSFXFile") == 0) {
        if (ctx->m_data_sound) {
            UI_NP_XML_READ_VALUE_STRING(ctx->m_data_sound->push_sfx_file);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "Modal") == 0) {
        if (ctx->m_data_window) {
            UI_NP_XML_READ_VALUE_BOOL(ctx->m_data_window->mudal);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "StartVisible") == 0) {
        if (ctx->m_data_window) {
            UI_NP_XML_READ_VALUE_BOOL(ctx->m_data_window->start_show);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "DialogTemplate") == 0) {
        if (ctx->m_data_window) {
            UI_NP_XML_READ_VALUE_BOOL(ctx->m_data_window->dialog_template);
        }
    }
}

static void ui_np_load_layout_structed_error(void * inputCtx, xmlErrorPtr err) {
    struct ui_np_load_layout_ctx * ctx = (struct ui_np_load_layout_ctx *)(inputCtx);

    CPE_ERROR_SET_LEVEL(
        ctx->m_em,
        err->level >= XML_ERR_ERROR ? CPE_EL_ERROR : CPE_EL_WARNING);

    CPE_ERROR_SET_LINE(ctx->m_em, err->line);

    cpe_error_do_notify(ctx->m_em, "(%d) %s", err->code, err->message);
}

static xmlSAXHandler g_ui_np_load_layout_callbacks = {
    NULL /* internalSubsetSAXFunc internalSubset */
    , NULL /* isStandaloneSAXFunc isStandalone */
    , NULL /* hasInternalSubsetSAXFunc hasInternalSubset */
    , NULL /* hasExternalSubsetSAXFunc hasExternalSubset */
    , NULL /* resolveEntitySAXFunc resolveEntity */
    , NULL /* getEntitySAXFunc getEntity */
    , NULL /* entityDeclSAXFunc entityDecl */
    , NULL /* notationDeclSAXFunc notationDecl */
    , NULL /* attributeDeclSAXFunc attributeDecl */
    , NULL /* elementDeclSAXFunc elementDecl */
    , NULL /* unparsedEntityDeclSAXFunc unparsedEntityDecl */
    , NULL /* setDocumentLocatorSAXFunc setDocumentLocator */
    , NULL /* startDocumentSAXFunc startDocument */
    , NULL /* endDocumentSAXFunc endDocument */
    , NULL /* startElementSAXFunc startElement */
    , NULL /* endElementSAXFunc endElement */
    , NULL /* referenceSAXFunc reference */
    , ui_np_load_layout_characters /* charactersSAXFunc characters */
    , NULL /* ignorableWhitespaceSAXFunc ignorableWhitespace */
    , NULL /* processingInstructionSAXFunc processingInstruction */
    , NULL /* commentSAXFunc comment */
    , NULL /* warningSAXFunc warning */
    , NULL /* errorSAXFunc error */
    , NULL /* fatalErrorSAXFunc fatalError; unused error() get all the errors * */
    , NULL /* getParameterEntitySAXFunc getParameterEntity */
    , NULL /* cdataBlockSAXFunc cdataBlock */
    , NULL /* externalSubsetSAXFunc externalSubset */
    , XML_SAX2_MAGIC /* unsigned int initialized */
    , NULL /* void *_private */
    , ui_np_load_layout_startElement /* startElementNsSAX2Func startElementNs */
    , ui_np_load_layout_endElement /* endElementNsSAX2Func endElementNs */
    , ui_np_load_layout_structed_error /* xmlStructuredErrorFunc serror */
};

void ui_data_np_load_layout_i(void * p, ui_data_mgr_t mgr, ui_data_src_t src, error_monitor_t em) {
    struct ui_np_load_layout_ctx ctx;
    struct mem_buffer path_buff;
    struct write_stream_buffer stream = CPE_WRITE_STREAM_BUFFER_INITIALIZER(&path_buff);
    const char * path;

    ctx.m_mgr = mgr;
    ctx.m_src = src;
    ctx.m_em = em;
    ctx.m_layout = ui_data_layout_create(mgr, src);
    if (ctx.m_layout == NULL) {
        CPE_ERROR(em, "create layout fail");
        return;
    }
    ctx.m_cur_control = NULL;

    mem_buffer_init(&path_buff, NULL);

    stream_printf((write_stream_t)&stream, "%s/", ui_data_src_data(ui_data_mgr_src_root(mgr)));
    ui_data_src_path_print((write_stream_t)&stream, src);
    stream_printf((write_stream_t)&stream, ".%s", ui_data_np_postfix(ui_data_src_type(src)));
    stream_putc((write_stream_t)&stream, 0);
    path = mem_buffer_make_continuous(&path_buff, 0);

    if (xmlSAXUserParseFile(&g_ui_np_load_layout_callbacks, &ctx, path) < 0) {
        CPE_ERROR(em, "parse fail!");
        ui_data_layout_free(ctx.m_layout);
        mem_buffer_clear(&path_buff);
        return;
    }

    mem_buffer_clear(&path_buff);
}

int ui_data_np_load_layout(void * ctx, ui_data_mgr_t mgr, ui_data_src_t src, error_monitor_t em) {
    int ret = 0;

    if (em) {
        CPE_DEF_ERROR_MONITOR_ADD(logError, em, cpe_error_save_last_errno, &ret);
        ui_data_np_load_layout_i(ctx, mgr, src, em);
        CPE_DEF_ERROR_MONITOR_REMOVE(logError, em);
    }
    else {
        CPE_DEF_ERROR_MONITOR(logError, cpe_error_save_last_errno, &ret);
        ui_data_np_load_layout_i(ctx, mgr, src, &logError);
    }

    return ret;
}

static void * ui_np_load_control_init_data_from_entry(
    struct ui_np_load_layout_ctx * ctx, const char * entry_name)
{
    LPDRMETA data_meta;
    LPDRMETAENTRY entry;

    if (ctx->m_cur_control == NULL) return NULL;

    data_meta = ui_data_control_meta(ctx->m_mgr, ui_data_control_type(ctx->m_cur_control));
    assert(data_meta);

    entry = dr_meta_find_entry_by_name(data_meta, entry_name);

    if (entry) {
        return ((char *)ui_data_control_data(ctx->m_cur_control)) + dr_entry_data_start_pos(entry, 0);
    }
    else {
        return NULL;
    }
}

static void ui_np_load_control_init_data(struct ui_np_load_layout_ctx * ctx) {
    ctx->m_data_basic = ui_np_load_control_init_data_from_entry(ctx, "basic");
    ctx->m_data_template = ui_np_load_control_init_data_from_entry(ctx, "tmpl");
    ctx->m_data_layout = ui_np_load_control_init_data_from_entry(ctx, "layout");
    ctx->m_data_state = ui_np_load_control_init_data_from_entry(ctx, "state");
    ctx->m_data_anim = ui_np_load_control_init_data_from_entry(ctx, "anim");
    ctx->m_data_sound = ui_np_load_control_init_data_from_entry(ctx, "sound");
    ctx->m_data_window = ui_np_load_control_init_data_from_entry(ctx, "window");
    ctx->m_data_ed = ui_np_load_control_init_data_from_entry(ctx, "ed");
}

static void ui_np_load_control_common_attrs(
    struct ui_np_load_layout_ctx * ctx,
    int nb_attributes,
    const xmlChar** attributes)
{

    if (ctx->m_data_basic) {
        uint32_t id;

        UI_NP_XML_READ_ATTR_INT(uint32_t, id, "ID");
        ui_data_control_set_id(ctx->m_cur_control, id);

        UI_NP_XML_READ_ATTR_STRING(ctx->m_data_basic->name, "Name");
    }

    if (ctx->m_data_template) {
    }
}
