#include <assert.h>
#include "libxml/xmlstring.h"
#include "libxml/parser.h"
#include "cpe/pal/pal_string.h"
#include "cpe/pal/pal_strings.h"
#include "ui/model/ui_data_action.h"
#include "ui/model/ui_data_src.h"
#include "ui_np_load_utils.h"

struct ui_np_load_action_ctx {
    ui_data_mgr_t m_mgr;
    ui_data_src_t m_src;
    error_monitor_t m_em;
    ui_data_action_t m_action;
    ui_data_actor_t m_cur_actor;
    UI_ACTOR * m_cur_actor_data;
    ui_data_actor_layer_t m_cur_layer;
    UI_ACTOR_LAYER * m_cur_layer_data;
    ui_data_actor_frame_t m_cur_frame;
    UI_ACTOR_FRAME * m_cur_frame_data;
    UI_IMG_REF * m_cur_img_ref;
    UI_FRAME_REF * m_cur_frame_ref;
    UI_TRANS * m_cur_trans;
    char m_cur_tag_name[64];
};

static void ui_np_load_action_startElement(
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
    struct ui_np_load_action_ctx * ctx = (struct ui_np_load_action_ctx *)(inputCtx);

    if (strcmp((const char *)localname, "Actor") == 0) {
        ctx->m_cur_actor = ui_data_actor_create(ctx->m_action);
        if (ctx->m_cur_actor  == NULL) {
            CPE_ERROR(ctx->m_em, "create actor fail!");
            ctx->m_cur_actor_data = NULL;
            return;
        }

        ctx->m_cur_actor_data = ui_data_actor_data(ctx->m_cur_actor);
        if (ctx->m_cur_actor_data) {
            UI_NP_XML_READ_ATTR_INT(uint32_t, ctx->m_cur_actor_data->id, "ID");
            UI_NP_XML_READ_ATTR_STRING(ctx->m_cur_actor_data->name, "Name");
        }
    }
    else if (strcmp((const char *)localname, "Layer") == 0) {
        if (ctx->m_cur_actor == NULL) return;

        ctx->m_cur_layer = ui_data_actor_layer_create(ctx->m_cur_actor);
        if (ctx->m_cur_layer == NULL) {
            CPE_ERROR(ctx->m_em, "create layer fail!");
            ctx->m_cur_layer_data = NULL;
            return;
        }

        ctx->m_cur_layer_data = ui_data_actor_layer_data(ctx->m_cur_layer);
        if (ctx->m_cur_layer_data) {
            UI_NP_XML_READ_ATTR_STRING(ctx->m_cur_layer_data->name, "Name");
            UI_NP_XML_READ_ATTR_BOOL(ctx->m_cur_layer_data->is_lead, "Lead");
        }
    }
    else if (strcmp((const char *)localname, "FrameRef") == 0) {
        if (ctx->m_cur_layer == NULL) return;

        ctx->m_cur_frame = ui_data_actor_frame_create(ctx->m_cur_layer);
        if (ctx->m_cur_frame == NULL) {
            CPE_ERROR(ctx->m_em, "create frame fail!");
            ctx->m_cur_frame_data = NULL;
            ctx->m_cur_img_ref = NULL;
            ctx->m_cur_frame_ref = NULL;
            ctx->m_cur_trans = NULL;
            return;
        }

        ctx->m_cur_frame_data = ui_data_actor_frame_data(ctx->m_cur_frame);
        if (ctx->m_cur_frame_data) {
            ctx->m_cur_frame_data->texture.type = UI_TEXTURE_REF_FRAME;
            ctx->m_cur_img_ref = NULL;
            ctx->m_cur_frame_ref = &ctx->m_cur_frame_data->texture.data.frame;
            ctx->m_cur_trans = &ctx->m_cur_frame_ref->trans;

            UI_NP_XML_READ_ATTR_INT(uint32_t, ctx->m_cur_frame_ref->sprite_id, "Resfile");
            UI_NP_XML_READ_ATTR_INT(uint32_t, ctx->m_cur_frame_ref->frame_id, "Frameid");
        }
    }
    else if (strcmp((const char *)localname, "ImageRef") == 0) {
        if (ctx->m_cur_layer == NULL) return;

        ctx->m_cur_frame = ui_data_actor_frame_create(ctx->m_cur_layer);
        if (ctx->m_cur_frame == NULL) {
            CPE_ERROR(ctx->m_em, "create frame fail!");
            ctx->m_cur_frame_data = NULL;
            ctx->m_cur_img_ref = NULL;
            ctx->m_cur_frame_ref = NULL;
            ctx->m_cur_trans = NULL;
            return;
        }

        ctx->m_cur_frame_data = ui_data_actor_frame_data(ctx->m_cur_frame);
        if (ctx->m_cur_frame_data) {
            ctx->m_cur_frame_data->texture.type = UI_TEXTURE_REF_IMG;
            ctx->m_cur_img_ref = &ctx->m_cur_frame_data->texture.data.img;
            ctx->m_cur_frame_ref = NULL;
            ctx->m_cur_trans = &ctx->m_cur_img_ref->trans;

            UI_NP_XML_READ_ATTR_INT(uint32_t, ctx->m_cur_img_ref->module_id, "Resfile");
            UI_NP_XML_READ_ATTR_INT(uint32_t, ctx->m_cur_img_ref->img_block_id, "Imageid");
            UI_NP_XML_READ_ATTR_STRING(ctx->m_cur_img_ref->name, "Resname");
        }
    }
    else if (strcmp((const char *)localname, "LocalAnglePivot") == 0) {
        if (ctx->m_cur_trans) {
            UI_NP_XML_READ_ATTR_VECTOR_3(&ctx->m_cur_trans->local_trans.angle_pivot);
        }
    }
    else if (strcmp((const char *)localname, "WorldAnglePivot") == 0) {
        if (ctx->m_cur_trans) {
            UI_NP_XML_READ_ATTR_VECTOR_3(&ctx->m_cur_trans->world_trans.angle_pivot);
        }
    }
    else if (strcmp((const char *)localname, "LocalScale") == 0) {
        if (ctx->m_cur_trans) {
            UI_NP_XML_READ_ATTR_VECTOR_3(&ctx->m_cur_trans->local_trans.scale);
        }
    }
    else if (strcmp((const char *)localname, "WorldScale") == 0) {
        if (ctx->m_cur_trans) {
            UI_NP_XML_READ_ATTR_VECTOR_3(&ctx->m_cur_trans->world_trans.scale);
        }
    }
    else if (strcmp((const char *)localname, "LocalTrans") == 0) {
        if (ctx->m_cur_trans) {
            UI_NP_XML_READ_ATTR_VECTOR_3(&ctx->m_cur_trans->local_trans.trans);
        }
    }
    else if (strcmp((const char *)localname, "WorldTrans") == 0) {
        if (ctx->m_cur_trans) {
            UI_NP_XML_READ_ATTR_VECTOR_3(&ctx->m_cur_trans->world_trans.trans);
        }
    }
    else if (strcmp((const char *)localname, "BColor") == 0) {
        if (ctx->m_cur_trans) {
            UI_NP_XML_READ_ATTR_COLOR(&ctx->m_cur_trans->background);
        }
    }
    else if (strcmp((const char *)localname, "OutlineColor") == 0) {
        if (ctx->m_cur_trans) {
            UI_NP_XML_READ_ATTR_COLOR(&ctx->m_cur_trans->ol.color);
        }
    }
    else if (strcmp((const char *)localname, "PolyVtx0") == 0) {
        if (ctx->m_cur_img_ref) {
            UI_NP_XML_READ_ATTR_VECTOR_2(&ctx->m_cur_img_ref->polys[0]);
        }
    }
    else if (strcmp((const char *)localname, "PolyVtx1") == 0) {
        if (ctx->m_cur_img_ref) {
            UI_NP_XML_READ_ATTR_VECTOR_2(&ctx->m_cur_img_ref->polys[1]);
        }
    }
    else if (strcmp((const char *)localname, "PolyVtx2") == 0) {
        if (ctx->m_cur_img_ref) {
            UI_NP_XML_READ_ATTR_VECTOR_2(&ctx->m_cur_img_ref->polys[2]);
        }
    }
    else if (strcmp((const char *)localname, "PolyVtx3") == 0) {
        if (ctx->m_cur_img_ref) {
            UI_NP_XML_READ_ATTR_VECTOR_2(&ctx->m_cur_img_ref->polys[3]);
        }
    }
    else {
        strncpy(ctx->m_cur_tag_name, (const char*)localname, sizeof(ctx->m_cur_tag_name));
    }
}

static void ui_np_load_action_endElement(
        void* inputCtx,
        const xmlChar* localname,
        const xmlChar* prefix,
        const xmlChar* URI)
{
    struct ui_np_load_action_ctx * ctx = (struct ui_np_load_action_ctx *)(inputCtx);

    ctx->m_cur_tag_name[0] = 0;

    if (strcmp((const char *)localname, "Actor") == 0) {
        ctx->m_cur_img_ref = NULL;
        ctx->m_cur_frame_ref = NULL;
        ctx->m_cur_trans = NULL;
        ctx->m_cur_frame = NULL;
        ctx->m_cur_frame_data = NULL;
        ctx->m_cur_layer = NULL;
        ctx->m_cur_layer_data = NULL;
        ctx->m_cur_actor = NULL;
        ctx->m_cur_actor_data = NULL;
    }
    else if (strcmp((const char *)localname, "Layer") == 0) {
        ctx->m_cur_img_ref = NULL;
        ctx->m_cur_frame_ref = NULL;
        ctx->m_cur_trans = NULL;
        ctx->m_cur_frame = NULL;
        ctx->m_cur_frame_data = NULL;
        ctx->m_cur_layer = NULL;
        ctx->m_cur_layer_data = NULL;
    }
    else if (strcmp((const char *)localname, "FrameRef") == 0
             || strcmp((const char *)localname, "ImageRef") == 0)
    {
        ctx->m_cur_img_ref = NULL;
        ctx->m_cur_frame_ref = NULL;
        ctx->m_cur_frame = NULL;
        ctx->m_cur_frame_data = NULL;
        ctx->m_cur_trans = NULL;
    }
}

static void ui_np_load_action_characters(void * inputCtx, const xmlChar *ch, int len) {
    struct ui_np_load_action_ctx * ctx = (struct ui_np_load_action_ctx *)(inputCtx);

    if (strcmp(ctx->m_cur_tag_name, "Time") == 0) {
        if (ctx->m_cur_frame_data) {
            UI_NP_XML_READ_VALUE_INT(uint16_t, ctx->m_cur_frame_data->start_frame);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "Sound") == 0) {
        if (ctx->m_cur_frame_data) {
            UI_NP_XML_READ_VALUE_STRING(ctx->m_cur_frame_data->sound);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "Smooth") == 0) {
        if (ctx->m_cur_frame_data) {
            UI_NP_XML_READ_VALUE_BOOL(ctx->m_cur_frame_data->smooth);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "Loop") == 0) {
        if (ctx->m_cur_actor_data) {
            UI_NP_XML_READ_VALUE_BOOL(ctx->m_cur_actor_data->is_loop);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "LoopStart") == 0) {
        if (ctx->m_cur_actor_data) {
            UI_NP_XML_READ_VALUE_INT(uint16_t, ctx->m_cur_actor_data->loop_start);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "LocalAngle") == 0) {
        if (ctx->m_cur_trans) {
            UI_NP_XML_READ_VALUE_FLOAT(ctx->m_cur_trans->local_trans.angle);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "WorldAngle") == 0) {
        if (ctx->m_cur_trans) {
            UI_NP_XML_READ_VALUE_FLOAT(ctx->m_cur_trans->world_trans.angle);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "LocalFlips") == 0) {
        if (ctx->m_cur_trans) {
            UI_NP_XML_READ_VALUE_INT(uint8_t, ctx->m_cur_trans->local_trans.flips);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "WorldFlips") == 0) {
        if (ctx->m_cur_trans) {
            UI_NP_XML_READ_VALUE_INT(uint8_t, ctx->m_cur_trans->world_trans.flips);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "Filter") == 0) {
        if (ctx->m_cur_trans) {
            UI_NP_XML_READ_VALUE_INT(uint32_t, ctx->m_cur_trans->filter);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "TexEnv") == 0) {
        if (ctx->m_cur_trans) {
            UI_NP_XML_READ_VALUE_INT(uint32_t, ctx->m_cur_trans->tex_env);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "SrcABM") == 0) {
        if (ctx->m_cur_trans) {
            UI_NP_XML_READ_VALUE_INT(uint32_t, ctx->m_cur_trans->src_abm);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "DstABM") == 0) {
        if (ctx->m_cur_trans) {
            UI_NP_XML_READ_VALUE_INT(uint32_t, ctx->m_cur_trans->dst_abm);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "Outline") == 0) {
        if (ctx->m_cur_trans) {
            UI_NP_XML_READ_VALUE_BOOL(ctx->m_cur_trans->ol.enable);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "OutlineWidth") == 0) {
        if (ctx->m_cur_trans) {
            UI_NP_XML_READ_VALUE_INT(uint8_t, ctx->m_cur_trans->ol.width);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "Freedom") == 0) {
        if (ctx->m_cur_img_ref) {
            UI_NP_XML_READ_VALUE_BOOL(ctx->m_cur_img_ref->freedom);
        }
    }
}

static void ui_np_load_action_structed_error(void * inputCtx, xmlErrorPtr err) {
    struct ui_np_load_action_ctx * ctx = (struct ui_np_load_action_ctx *)(inputCtx);

    if (err->code == XML_ERR_DOCUMENT_END) {
        ((xmlParserCtxtPtr)err->ctxt)->wellFormed = 1;
        xmlCtxtResetLastError(err->ctxt);
    }
    else {
        CPE_ERROR_SET_LEVEL(
            ctx->m_em,
            err->level >= XML_ERR_ERROR ? CPE_EL_ERROR : CPE_EL_WARNING);

        CPE_ERROR_SET_LINE(ctx->m_em, err->line);

        cpe_error_do_notify(ctx->m_em, "(%d) %s", err->code, err->message);
    }
}

static xmlSAXHandler g_ui_np_load_action_callbacks = {
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
    , ui_np_load_action_characters /* charactersSAXFunc characters */
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
    , ui_np_load_action_startElement /* startElementNsSAX2Func startElementNs */
    , ui_np_load_action_endElement /* endElementNsSAX2Func endElementNs */
    , ui_np_load_action_structed_error /* xmlStructuredErrorFunc serror */
};

void ui_data_np_load_action_i(void * p, ui_data_mgr_t mgr, ui_data_src_t src, error_monitor_t em) {
    struct ui_np_load_action_ctx ctx;
    struct mem_buffer data_buff;
    const char * data;
    const char * data_start;
    const char * sep1 = "</FrameFile>";
    const char * sep2 = "<FrameFile />";

    ctx.m_mgr = mgr;
    ctx.m_src = src;
    ctx.m_em = em;
    ctx.m_action = NULL;
    ctx.m_cur_actor = NULL;
    ctx.m_cur_layer = NULL;
    ctx.m_cur_frame = NULL;
    ctx.m_cur_img_ref = NULL;
    ctx.m_cur_frame_ref = NULL;
    ctx.m_cur_trans = NULL;

    mem_buffer_init(&data_buff, NULL);
    data = ui_data_np_load_src_to_buff(&data_buff, src, em);
    if (data == NULL) {
        mem_buffer_clear(&data_buff);
        return;
    }

    data_start = strstr(data, sep1);
    if (data_start == NULL) {
        data_start = strstr(data, sep2);
        if (data_start) {
            data_start += strlen(sep2);
        }
        else {
            data_start = data;
        }
    }
    else {
        data_start += strlen(sep1);
    }

    ctx.m_action = ui_data_action_create(mgr, src);
    if (ctx.m_action == NULL) {
        CPE_ERROR(em, "create action fail");
        mem_buffer_clear(&data_buff);
        return;
    }

    xmlSAXUserParseMemory(&g_ui_np_load_action_callbacks, &ctx, data_start, strlen(data_start));

    ui_data_action_update_refs(ctx.m_action);

    mem_buffer_clear(&data_buff);
}

int ui_data_np_load_action(void * ctx, ui_data_mgr_t mgr, ui_data_src_t src, error_monitor_t em) {
    int ret = 0;

    if (em) {
        CPE_DEF_ERROR_MONITOR_ADD(logError, em, cpe_error_save_last_errno, &ret);
        ui_data_np_load_action_i(ctx, mgr, src, em);
        CPE_DEF_ERROR_MONITOR_REMOVE(logError, em);
    }
    else {
        CPE_DEF_ERROR_MONITOR(logError, cpe_error_save_last_errno, &ret);
        ui_data_np_load_action_i(ctx, mgr, src, &logError);
    }

    return ret;
}
