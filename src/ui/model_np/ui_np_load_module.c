#include <assert.h>
#include "libxml/xmlstring.h"
#include "libxml/parser.h"
#include "cpe/pal/pal_string.h"
#include "cpe/pal/pal_strings.h"
#include "ui/model/ui_data_module.h"
#include "ui/model/ui_data_src.h"
#include "ui_np_load_utils.h"

struct ui_np_load_module_ctx {
    ui_data_mgr_t m_mgr;
    ui_data_src_t m_src;
    error_monitor_t m_em;
    ui_data_module_t m_module;
    ui_data_img_block_t m_cur_img_block;
};

static void ui_np_load_module_startElement(
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
    struct ui_np_load_module_ctx * ctx = (struct ui_np_load_module_ctx *)(inputCtx);
    UI_IMG_BLOCK * block = ctx->m_cur_img_block ? ui_data_img_block_data(ctx->m_cur_img_block) : NULL;

    if (strcmp((const char *)localname, "Image") == 0) {
        uint32_t id;

        ctx->m_cur_img_block = ui_data_img_block_create(ctx->m_module);
        if (ctx->m_cur_img_block  == NULL) {
            CPE_ERROR(ctx->m_em, "create img block fail!");
            return;
        }

        UI_NP_XML_READ_ATTR_INT(uint32_t, id, "ID");
        if (ui_data_img_block_set_id(ctx->m_cur_img_block, id) != 0 ) {
            CPE_ERROR(ctx->m_em, "create img block: id duplicate!");
            return;
        }

        block = ui_data_img_block_data(ctx->m_cur_img_block);

        UI_NP_XML_READ_ATTR_STRING(block->name, "Name");

        UI_NP_XML_READ_ATTR_INT(uint32_t, block->src_x, "SrcX");
        UI_NP_XML_READ_ATTR_INT(uint32_t, block->src_y, "SrcY");

        UI_NP_XML_READ_ATTR_INT(uint32_t, block->src_w, "SrcW");
        UI_NP_XML_READ_ATTR_INT(uint32_t, block->src_h, "SrcH");

        UI_NP_XML_READ_ATTR_INT(uint32_t, block->flag, "Flag");
    }
    else if (strcmp((const char *)localname, "Rect") == 0) {
        if (block) {
            UI_NP_XML_READ_ATTR_RECT(&block->rect);
        }
    }
    else if (strcmp((const char *)localname, "SCTex") == 0) {
        UI_MODULE *  module_data = ui_data_module_data(ctx->m_module);
        int write_pos;
        char buff[256];
 
        UI_NP_XML_READ_ATTR_STRING(module_data->use_img, "Path");
        write_pos = strlen(module_data->use_img);

        UI_NP_XML_READ_ATTR_STRING(buff, "File");
        snprintf(module_data->use_img + write_pos, sizeof(module_data->use_img) - write_pos, "/%s", buff);
    }
}

static void ui_np_load_module_endElement(
        void* inputCtx,
        const xmlChar* localname,
        const xmlChar* prefix,
        const xmlChar* URI)
{
    struct ui_np_load_module_ctx * ctx = (struct ui_np_load_module_ctx *)(inputCtx);

    if (strcmp((const char *)localname, "Image") == 0) {
        ctx->m_cur_img_block = NULL;
    }
}

static void ui_np_load_module_characters(void * inputCtx, const xmlChar *ch, int len) {
}

static void ui_np_load_module_structed_error(void * inputCtx, xmlErrorPtr err) {
    struct ui_np_load_module_ctx * ctx = (struct ui_np_load_module_ctx *)(inputCtx);

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

static xmlSAXHandler g_ui_np_load_module_callbacks = {
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
    , ui_np_load_module_characters /* charactersSAXFunc characters */
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
    , ui_np_load_module_startElement /* startElementNsSAX2Func startElementNs */
    , ui_np_load_module_endElement /* endElementNsSAX2Func endElementNs */
    , ui_np_load_module_structed_error /* xmlStructuredErrorFunc serror */
};

void ui_data_np_load_module_i(void * p, ui_data_mgr_t mgr, ui_data_src_t src, error_monitor_t em) {
    struct ui_np_load_module_ctx ctx;
    struct mem_buffer data_buff;
    const char * data;
    const char * data_end;
    size_t data_size;
    const char * sep = "</SCTexList>";
    size_t sep_len = strlen(sep);

    ctx.m_mgr = mgr;
    ctx.m_src = src;
    ctx.m_em = em;
    ctx.m_module = NULL;
    ctx.m_cur_img_block = NULL;

    mem_buffer_init(&data_buff, NULL);
    data = ui_data_np_load_src_to_buff(&data_buff, src, em);
    if (data == NULL) {
        mem_buffer_clear(&data_buff);
        return;
    }

    data_end = strstr(data, sep);
    if (data_end == NULL) {
        CPE_ERROR(em, "find %s fail!", sep);
        mem_buffer_clear(&data_buff);
        return;
    }
    data_size = (data_end - data) + sep_len;

    ctx.m_module = ui_data_module_create(mgr, src);
    if (ctx.m_module == NULL) {
        CPE_ERROR(em, "create module fail");
        mem_buffer_clear(&data_buff);
        return;
    }

    xmlSAXUserParseMemory(&g_ui_np_load_module_callbacks, &ctx, data, data_size);

    xmlSAXUserParseMemory(&g_ui_np_load_module_callbacks, &ctx, data + data_size, mem_buffer_size(&data_buff) - data_size - 1);
    
    mem_buffer_clear(&data_buff);
}

int ui_data_np_load_module(void * ctx, ui_data_mgr_t mgr, ui_data_src_t src, error_monitor_t em) {
    int ret = 0;

    if (em) {
        CPE_DEF_ERROR_MONITOR_ADD(logError, em, cpe_error_save_last_errno, &ret);
        ui_data_np_load_module_i(ctx, mgr, src, em);
        CPE_DEF_ERROR_MONITOR_REMOVE(logError, em);
    }
    else {
        CPE_DEF_ERROR_MONITOR(logError, cpe_error_save_last_errno, &ret);
        ui_data_np_load_module_i(ctx, mgr, src, &logError);
    }

    return ret;
}
