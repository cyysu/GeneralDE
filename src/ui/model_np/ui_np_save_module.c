#include <assert.h>
#include "cpe/pal/pal_string.h"
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/buffer.h"
#include "cpe/utils/stream_file.h"
#include "ui/model/ui_data_module.h"
#include "ui/model/ui_data_src.h"
#include "ui/model/ui_data_mgr.h"
#include "ui_np_save_utils.h"
#include "ui_np_utils.h"

static int ui_data_np_save_module_i(write_stream_t s, ui_data_module_t module, error_monitor_t em) {
    struct mem_buffer buff;
    struct ui_data_img_block_it img_block_it;
    ui_data_img_block_t img_block;
    UI_MODULE *  module_data = ui_data_module_data(module);
    char * sep;

    mem_buffer_init(&buff, NULL);

    stream_printf(s, "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n");

    stream_printf(s, "<SCTexList SCTexAuto=\"True\" SCTexAsyn=\"False\" SCTexManaged=\"True\" SCTexIndex=\"0\">\n");
    if ((sep = strrchr(module_data->use_img, '/'))) {
        stream_printf(s, "    <SCTex Path=\"");
        stream_write(s, module_data->use_img, sep - module_data->use_img);
        stream_printf(s, "/\" File=\"%s\" />\n", sep + 1);
    }
    else {
        stream_printf(s, "    <SCTex Path=\"/\" File=\"%s\" />\n", module_data->use_img);
    }
    stream_printf(s, "</SCTexList>\n");

    stream_printf(s, "<ImageList>\n");
    ui_data_img_block_in_module(&img_block_it, module);
    while((img_block = ui_data_img_block_it_next(&img_block_it))) {
        UI_IMG_BLOCK const * block_data = ui_data_img_block_data(img_block);
        
        stream_printf(s, "    <Image ID=\"%u\" Name=\"%s\" SrcX=\"%d\" SrcY=\"%d\" SrcW=\"%d\" SrcH=\"%d\" Flag=\"%d\">\n",
                      block_data->id, block_data->name,
                      block_data->src_x, block_data->src_y, block_data->src_w, block_data->src_h, block_data->flag);
        ui_data_np_save_rect(s, 2, "Rect", &block_data->rect);
        stream_printf(s, "    </Image>\n");
    }
    stream_printf(s, "</ImageList>\n");

    mem_buffer_clear(&buff);
    return 0;
}

int ui_data_np_save_module(void * ctx, ui_data_mgr_t mgr, ui_data_src_t src, const char * root, error_monitor_t em) {
    FILE * fp = NULL;
    int rv = -1;
    struct write_stream_file fs;

    fp = ui_data_np_save_open_file(root, src, em);
    if (fp == NULL) goto COMPLETE;

    if (ui_data_np_save_gen_meta_file(root, src, em) != 0) goto COMPLETE;

    write_stream_file_init(&fs, fp, em);
    rv = ui_data_np_save_module_i((write_stream_t)&fs, ui_data_src_product(src), em);

COMPLETE:
    if (fp) file_stream_close(fp, em);
    return rv;
}
