#include <assert.h>
#include "cpe/utils/buffer.h"
#include "cpe/utils/stream_file.h"
#include "ui/model/ui_data_sprite.h"
#include "ui/model/ui_data_src.h"
#include "ui/model/ui_data_mgr.h"
#include "ui_np_save_utils.h"
#include "ui_np_utils.h"

static int ui_data_np_save_sprite_i(write_stream_t s, ui_data_sprite_t sprite, error_monitor_t em) {
    struct ui_data_src_ref_it src_ref_it;
    ui_data_src_ref_t src_ref;
    struct ui_data_frame_it frame_it;
    ui_data_frame_t frame;
    struct ui_data_frame_img_it img_ref_it;
    ui_data_frame_img_t img_ref;

    stream_printf(s, "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n");

    ui_data_sprite_use_refs(&src_ref_it, sprite);
    src_ref = ui_data_src_ref_it_next(&src_ref_it);
    if (src_ref == NULL) {
        stream_printf(s, "<ImageFile />\n");
    }
    else {
        stream_printf(s, "<ImageFile>\n");
        for(; src_ref; src_ref = ui_data_src_ref_it_next(&src_ref_it)) {
            stream_printf(s, "    <File Resfile=\"%u\" />\n", ui_data_src_ref_using_src_id(src_ref));
        }
        stream_printf(s, "</ImageFile>\n");
    }

    ui_data_sprite_frames(&frame_it, sprite);
    frame = ui_data_frame_it_next(&frame_it);
    if (frame == NULL) {
        stream_printf(s, "<FrameList />\n");
    }
    else {
        stream_printf(s, "<FrameList>\n");
        for(; frame; frame = ui_data_frame_it_next(&frame_it)) {
            UI_FRAME const * frame_data = ui_data_frame_data(frame);

            stream_printf(s, "    <Frame ID=\"%u\" Name=\"%s\">\n", frame_data->id, frame_data->name);
            ui_data_np_save_rect(s, 2, "Bounding", &frame_data->bounding);
            ui_data_np_save_bool(s, 2, "BoundCustom", frame_data->bound_custom);
            ui_data_np_save_bool(s, 2, "AcceptScale", frame_data->accept_scale);
            ui_data_np_save_bool(s, 2, "MulLanguage", 0);

            ui_data_frame_imgs(&img_ref_it, frame);
            img_ref = ui_data_frame_img_it_next(&img_ref_it);
            if (img_ref == NULL) {
                stream_printf(s, "        <ImageRefList />\n");
            }
            else {
                stream_printf(s, "        <ImageRefList>\n");
                for(; img_ref; img_ref = ui_data_frame_img_it_next(&img_ref_it)) {
                    UI_IMG_REF const * img_ref_data = ui_data_frame_img_data(img_ref);

                    stream_printf(s, "            <ImageRef Resfile=\"%u\" Imageid=\"%u\" Resname=\"%s\">\n",
                                  img_ref_data->module_id, img_ref_data->img_block_id, img_ref_data->name);
                    ui_data_np_save_trans(s, 4, &img_ref_data->trans);
                    ui_data_np_save_bool(s, 4, "Freedom", img_ref_data->freedom);
                    if (img_ref_data->freedom) {
                        ui_data_np_save_vector_2(s, 4, "PolyVtx0", img_ref_data->polys + 0);
                        ui_data_np_save_vector_2(s, 4, "PolyVtx1", img_ref_data->polys + 1);
                        ui_data_np_save_vector_2(s, 4, "PolyVtx2", img_ref_data->polys + 2);
                        ui_data_np_save_vector_2(s, 4, "PolyVtx3", img_ref_data->polys + 3);
                    }
                    stream_printf(s, "            </ImageRef>\n");
                }
                stream_printf(s, "        </ImageRefList>\n");
            }

            stream_printf(s, "        <ColliderList />\n");

            stream_printf(s, "    </Frame>\n");
        }
        stream_printf(s, "</FrameList>\n");
    }

    return 0;
}

int ui_data_np_save_sprite(void * ctx, ui_data_mgr_t mgr, ui_data_src_t src, const char * root, error_monitor_t em) {
    FILE * fp = NULL;
    int rv = -1;
    struct write_stream_file fs;

    fp = ui_data_np_save_open_file(root, src, em);
    if (fp == NULL) goto COMPLETE;

    if (ui_data_np_save_gen_meta_file(root, src, em) != 0) goto COMPLETE;

    write_stream_file_init(&fs, fp, em);
    rv = ui_data_np_save_sprite_i((write_stream_t)&fs, ui_data_src_product(src), em);

COMPLETE:
    if (fp) file_stream_close(fp, em);
    return rv;
}
