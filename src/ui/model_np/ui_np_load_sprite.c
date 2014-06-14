#include <assert.h>
#include "libxml/xmlstring.h"
#include "libxml/parser.h"
#include "cpe/pal/pal_string.h"
#include "cpe/pal/pal_strings.h"
#include "ui/model/ui_data_sprite.h"
#include "ui/model/ui_data_src.h"
#include "ui_np_load_utils.h"

struct ui_np_load_sprite_ctx {
    ui_data_mgr_t m_mgr;
    ui_data_src_t m_src;
    error_monitor_t m_em;
    ui_data_sprite_t m_sprite;
    ui_data_frame_t m_cur_frame;
    ui_data_frame_img_t m_cur_img_ref;
    char m_cur_tag_name[64];
};

static void ui_np_load_sprite_startElement(
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
    struct ui_np_load_sprite_ctx * ctx = (struct ui_np_load_sprite_ctx *)(inputCtx);
    UI_FRAME * frame = ctx->m_cur_frame ? ui_data_frame_data(ctx->m_cur_frame) : NULL;
    UI_IMG_REF * img_ref_data = ctx->m_cur_img_ref ? ui_data_frame_img_data(ctx->m_cur_img_ref) : NULL;

    if (strcmp((const char *)localname, "Frame") == 0) {
        uint32_t id;

        UI_NP_XML_READ_ATTR_INT(uint32_t, id, "ID");

        ctx->m_cur_frame = ui_data_frame_create(ctx->m_sprite, id);
        if (ctx->m_cur_frame  == NULL) {
            CPE_ERROR(ctx->m_em, "create frame fail!");
            return;
        }

        frame = ui_data_frame_data(ctx->m_cur_frame);
        UI_NP_XML_READ_ATTR_STRING(frame->name, "Name");
    }
    else if (strcmp((const char *)localname, "ImageRef") == 0) {
        if (ctx->m_cur_frame == NULL) return;

        ctx->m_cur_img_ref = ui_data_frame_img_create(ctx->m_cur_frame);
        if (ctx->m_cur_img_ref == NULL) {
            CPE_ERROR(ctx->m_em, "create img ref fail!");
            return;
        }

        img_ref_data = ui_data_frame_img_data(ctx->m_cur_img_ref);
        UI_NP_XML_READ_ATTR_INT(uint32_t, img_ref_data->module_id, "Resfile");
        UI_NP_XML_READ_ATTR_INT(uint32_t, img_ref_data->img_block_id, "Imageid");
        UI_NP_XML_READ_ATTR_STRING(img_ref_data->name, "Resname");
    }
    else if (strcmp((const char *)localname, "Bounding") == 0) {
        if (frame) {
            UI_NP_XML_READ_ATTR_RECT(&frame->bounding);
        }
    }
    else if (strcmp((const char *)localname, "LocalAnglePivot") == 0) {
        if (img_ref_data) {
            UI_NP_XML_READ_ATTR_VECTOR_3(&img_ref_data->trans.local_trans.angle_pivot);
        }
    }
    else if (strcmp((const char *)localname, "WorldAnglePivot") == 0) {
        if (img_ref_data) {
            UI_NP_XML_READ_ATTR_VECTOR_3(&img_ref_data->trans.world_trans.angle_pivot);
        }
    }
    else if (strcmp((const char *)localname, "LocalScale") == 0) {
        if (img_ref_data) {
            UI_NP_XML_READ_ATTR_VECTOR_3(&img_ref_data->trans.local_trans.scale);
        }
    }
    else if (strcmp((const char *)localname, "WorldScale") == 0) {
        if (img_ref_data) {
            UI_NP_XML_READ_ATTR_VECTOR_3(&img_ref_data->trans.world_trans.scale);
        }
    }
    else if (strcmp((const char *)localname, "LocalTrans") == 0) {
        if (img_ref_data) {
            UI_NP_XML_READ_ATTR_VECTOR_3(&img_ref_data->trans.local_trans.trans);
        }
    }
    else if (strcmp((const char *)localname, "WorldTrans") == 0) {
        if (img_ref_data) {
            UI_NP_XML_READ_ATTR_VECTOR_3(&img_ref_data->trans.world_trans.trans);
        }
    }
    else if (strcmp((const char *)localname, "BColor") == 0) {
        if (img_ref_data) {
            UI_NP_XML_READ_ATTR_COLOR(&img_ref_data->trans.background);
        }
    }
    else if (strcmp((const char *)localname, "OutlineColor") == 0) {
        if (img_ref_data) {
            UI_NP_XML_READ_ATTR_COLOR(&img_ref_data->trans.ol.color);
        }
    }
    else if (strcmp((const char *)localname, "PolyVtx0") == 0) {
        if (img_ref_data) {
            UI_NP_XML_READ_ATTR_VECTOR_2(&img_ref_data->polys[0]);
        }
    }
    else if (strcmp((const char *)localname, "PolyVtx1") == 0) {
        if (img_ref_data) {
            UI_NP_XML_READ_ATTR_VECTOR_2(&img_ref_data->polys[1]);
        }
    }
    else if (strcmp((const char *)localname, "PolyVtx2") == 0) {
        if (img_ref_data) {
            UI_NP_XML_READ_ATTR_VECTOR_2(&img_ref_data->polys[2]);
        }
    }
    else if (strcmp((const char *)localname, "PolyVtx3") == 0) {
        if (img_ref_data) {
            UI_NP_XML_READ_ATTR_VECTOR_2(&img_ref_data->polys[3]);
        }
    }
    else {
        strncpy(ctx->m_cur_tag_name, (const char*)localname, sizeof(ctx->m_cur_tag_name));
    }
}

static void ui_np_load_sprite_endElement(
        void* inputCtx,
        const xmlChar* localname,
        const xmlChar* prefix,
        const xmlChar* URI)
{
    struct ui_np_load_sprite_ctx * ctx = (struct ui_np_load_sprite_ctx *)(inputCtx);

    ctx->m_cur_tag_name[0] = 0;

    if (strcmp((const char *)localname, "Frame") == 0) {
        ctx->m_cur_frame = NULL;
        ctx->m_cur_img_ref = NULL;
    }
    else if (strcmp((const char *)localname, "ImageRef") == 0) {
        ctx->m_cur_img_ref = NULL;
    }
}

static void ui_np_load_sprite_characters(void * inputCtx, const xmlChar *ch, int len) {
    struct ui_np_load_sprite_ctx * ctx = (struct ui_np_load_sprite_ctx *)(inputCtx);
    UI_FRAME * frame = ctx->m_cur_frame ? ui_data_frame_data(ctx->m_cur_frame) : NULL;
    UI_IMG_REF * img_ref_data = ctx->m_cur_img_ref ? ui_data_frame_img_data(ctx->m_cur_img_ref) : NULL;

    if (strcmp(ctx->m_cur_tag_name, "BoundCustom") == 0) {
        if (frame) {
            UI_NP_XML_READ_VALUE_BOOL(frame->bound_custom);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "AcceptScale") == 0) {
        if (frame) {
            UI_NP_XML_READ_VALUE_BOOL(frame->accept_scale);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "LocalAngle") == 0) {
        if (img_ref_data) {
            UI_NP_XML_READ_VALUE_FLOAT(img_ref_data->trans.local_trans.angle);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "WorldAngle") == 0) {
        if (img_ref_data) {
            UI_NP_XML_READ_VALUE_FLOAT(img_ref_data->trans.world_trans.angle);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "LocalFlips") == 0) {
        if (img_ref_data) {
            UI_NP_XML_READ_VALUE_INT(uint8_t, img_ref_data->trans.local_trans.flips);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "WorldFlips") == 0) {
        if (img_ref_data) {
            UI_NP_XML_READ_VALUE_INT(uint8_t, img_ref_data->trans.world_trans.flips);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "Filter") == 0) {
        if (img_ref_data) {
            UI_NP_XML_READ_VALUE_INT(uint32_t, img_ref_data->trans.filter);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "TexEnv") == 0) {
        if (img_ref_data) {
            UI_NP_XML_READ_VALUE_INT(uint32_t, img_ref_data->trans.tex_env);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "SrcABM") == 0) {
        if (img_ref_data) {
            UI_NP_XML_READ_VALUE_INT(uint32_t, img_ref_data->trans.src_abm);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "DstABM") == 0) {
        if (img_ref_data) {
            UI_NP_XML_READ_VALUE_INT(uint32_t, img_ref_data->trans.dst_abm);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "Outline") == 0) {
        if (img_ref_data) {
            UI_NP_XML_READ_VALUE_BOOL(img_ref_data->trans.ol.enable);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "OutlineWidth") == 0) {
        if (img_ref_data) {
            UI_NP_XML_READ_VALUE_INT(uint32_t, img_ref_data->trans.ol.width);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "Freedom") == 0) {
        if (img_ref_data) {
            UI_NP_XML_READ_VALUE_BOOL(img_ref_data->freedom);
        }
    }
}

static void ui_np_load_sprite_structed_error(void * inputCtx, xmlErrorPtr err) {
    struct ui_np_load_sprite_ctx * ctx = (struct ui_np_load_sprite_ctx *)(inputCtx);

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

static xmlSAXHandler g_ui_np_load_sprite_callbacks = {
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
    , ui_np_load_sprite_characters /* charactersSAXFunc characters */
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
    , ui_np_load_sprite_startElement /* startElementNsSAX2Func startElementNs */
    , ui_np_load_sprite_endElement /* endElementNsSAX2Func endElementNs */
    , ui_np_load_sprite_structed_error /* xmlStructuredErrorFunc serror */
};

void ui_data_np_load_sprite_i(void * p, ui_data_mgr_t mgr, ui_data_src_t src, error_monitor_t em) {
    struct ui_np_load_sprite_ctx ctx;
    struct mem_buffer data_buff;
    const char * data;
    const char * data_start;
    const char * sep1 = "</ImageFile>";
    const char * sep2 = "<ImageFile />";

    ctx.m_mgr = mgr;
    ctx.m_src = src;
    ctx.m_em = em;
    ctx.m_sprite = NULL;
    ctx.m_cur_frame = NULL;
    ctx.m_cur_img_ref = NULL;

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

    ctx.m_sprite = ui_data_sprite_create(mgr, src);
    if (ctx.m_sprite == NULL) {
        CPE_ERROR(em, "create sprite fail");
        mem_buffer_clear(&data_buff);
        return;
    }

    xmlSAXUserParseMemory(&g_ui_np_load_sprite_callbacks, &ctx, data_start, strlen(data_start));

    ui_data_sprite_update_refs(ctx.m_sprite);

    mem_buffer_clear(&data_buff);
}

int ui_data_np_load_sprite(void * ctx, ui_data_mgr_t mgr, ui_data_src_t src, error_monitor_t em) {
    int ret = 0;

    if (em) {
        CPE_DEF_ERROR_MONITOR_ADD(logError, em, cpe_error_save_last_errno, &ret);
        ui_data_np_load_sprite_i(ctx, mgr, src, em);
        CPE_DEF_ERROR_MONITOR_REMOVE(logError, em);
    }
    else {
        CPE_DEF_ERROR_MONITOR(logError, cpe_error_save_last_errno, &ret);
        ui_data_np_load_sprite_i(ctx, mgr, src, &logError);
    }

    return ret;
}
