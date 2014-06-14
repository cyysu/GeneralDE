#include <assert.h>
#include "cpe/pal/pal_string.h"
#include "cpe/utils/buffer.h"
#include "cpe/utils/stream_file.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "ui/model/ui_data_layout.h"
#include "ui/model/ui_data_src.h"
#include "ui/model/ui_data_mgr.h"
#include "ui_np_save_utils.h"
#include "ui_np_utils.h"

struct ui_np_save_control_ctx {
    ui_data_control_t m_control;
    UI_CONTROL_BASIC * m_data_basic;
    UI_CONTROL_TEMPLATE * m_data_template;
    UI_CONTROL_LAYOUT * m_data_layout;
    UI_CONTROL_STATE * m_data_state;
    UI_CONTROL_ANIM * m_data_anim;
    UI_CONTROL_SOUND * m_data_sound;
    UI_CONTROL_ED * m_data_ed;
    UI_CONTROL_WINDOW * m_data_window;
};

static void * ui_np_control_data_from_entry(ui_data_mgr_t mgr, ui_data_control_t control, const char * entry_name);
static const char * ui_data_np_control_type_name(ui_data_control_type_t control_type);

static int ui_data_np_save_control_i(
    write_stream_t s, ui_data_mgr_t mgr, ui_data_control_t control, uint8_t level, error_monitor_t em)
{
    struct ui_data_control_it control_it;
    ui_data_control_t child_control;
    struct ui_np_save_control_ctx ctx;
    int rv = 0;
    const char * tag_name;

    /*得到TagName */
    tag_name = ui_data_np_control_tag_name(ui_data_control_type(control));
    if (tag_name == NULL) {
        CPE_ERROR(em, "ui_data_np_save_control: not support type %d", ui_data_control_type(control));
        return -1;
    }

    if (level + 1 > 64) {
        CPE_ERROR(em, "ui_data_np_save_control: level %d overflow!", level);
        return -1;
    }

    ctx.m_control = control;
    ctx.m_data_basic = ui_np_control_data_from_entry(mgr, control, "basic");
    ctx.m_data_template = ui_np_control_data_from_entry(mgr, control, "tmpl");
    ctx.m_data_layout = ui_np_control_data_from_entry(mgr, control, "layout");
    ctx.m_data_state = ui_np_control_data_from_entry(mgr, control, "state");
    ctx.m_data_anim = ui_np_control_data_from_entry(mgr, control, "anim");
    ctx.m_data_sound = ui_np_control_data_from_entry(mgr, control, "sound");
    ctx.m_data_ed = ui_np_control_data_from_entry(mgr, control, "ed");
    ctx.m_data_window = ui_np_control_data_from_entry(mgr, control, "window");
    
    stream_putc_count(s, ' ', level * 4);
    stream_printf(
        s, "<%s ID=\"%u\" Name=\"%s\" Control=\"%s\"",
        tag_name,
        ctx.m_data_basic->id, ctx.m_data_basic->name,
        ui_data_np_control_type_name(ui_data_control_type(control)));
    if (ctx.m_data_template) {
        stream_printf(
            s, " TemplateLink=\"%s\" TemplateLinkCtrl=\"%s\"",
            ctx.m_data_template->is_link ? "True" : "False",
            ctx.m_data_template->link_control);
    }
    stream_printf(s, ">\n");

    ui_data_np_save_unit_vector_2(s, level + 1, "RenderPT", &ctx.m_data_layout->render_pt);
    ui_data_np_save_vector_2(s, level + 1, "EditorPT", &ctx.m_data_ed->editor_pt);
    ui_data_np_save_str(s, level + 1, "UserText", ctx.m_data_basic->user_text);
    ui_data_np_save_int(s, level + 1, "AlignHorz", ctx.m_data_layout->align_horz);
    ui_data_np_save_int(s, level + 1, "AlignVert", ctx.m_data_layout->align_vert);
    ui_data_np_save_bool(s, level + 1, "Visible", ctx.m_data_state->visiable);

    if (!ctx.m_data_template->is_link) {
        ui_data_np_save_bool(s, level + 1, "Template", 0);
        ui_data_np_save_vector_2(s, level + 1, "EditorSZ", &ctx.m_data_ed->editor_sz);
        ui_data_np_save_rect(s, level + 1, "EditorPD", &ctx.m_data_ed->editor_pd);
        ui_data_np_save_unit_vector_2(s, level + 1, "RenderSZ", &ctx.m_data_layout->render_sz);
        ui_data_np_save_unit_rect(s, level + 1, "ClientPD", &ctx.m_data_layout->client_pd);
        ui_data_np_save_vector_2(s, level + 1, "Pivot", &ctx.m_data_layout->pivot);
        ui_data_np_save_vector_2(s, level + 1, "Scale", &ctx.m_data_layout->scale);
        ui_data_np_save_vector_3(s, level + 1, "Angle", &ctx.m_data_layout->angle);
        ui_data_np_save_float(s, level + 1, "Alpha", ctx.m_data_layout->alpha);
        ui_data_np_save_color(s, level + 1, "Color", &ctx.m_data_layout->color);
        ui_data_np_save_color(s, level + 1, "GrayColor", &ctx.m_data_layout->gray_color);
        ui_data_np_save_int(s, level + 1, "DrawAlign", ctx.m_data_layout->draw_align);
        /* 	NPXmlHelper::ExportArray(node, NPGUIKey::BackFrame,			mBackFrame); */
        /*  NPXmlHelper::ExportArray(node, NPGUIKey::ForeFrame,         mForeFrame); */
        ui_data_np_save_bool(s, level + 1, "DrawColor", ctx.m_data_layout->draw_color);
        ui_data_np_save_bool(s, level + 1, "DrawFrame", ctx.m_data_layout->draw_frame);
        ui_data_np_save_bool(s, level + 1, "DrawInner", ctx.m_data_layout->draw_inner);
        ui_data_np_save_bool(s, level + 1, "Enable", ctx.m_data_state->enable);
        ui_data_np_save_bool(s, level + 1, "AlwaysTop", ctx.m_data_state->always_top);
        ui_data_np_save_bool(s, level + 1, "AcceptPTLS", ctx.m_data_state->accept_pt_ls);
        ui_data_np_save_bool(s, level + 1, "AcceptSZLS", ctx.m_data_state->accept_sz_ls);
        ui_data_np_save_bool(s, level + 1, "AcceptHits", ctx.m_data_state->accept_hits);
        ui_data_np_save_bool(s, level + 1, "AcceptClip", ctx.m_data_state->accept_clip);
        ui_data_np_save_bool(s, level + 1, "ParentClip", ctx.m_data_state->parent_clip);
        ui_data_np_save_bool(s, level + 1, "AcceptMove", ctx.m_data_state->accept_move);
        /* 	NPXmlHelper::ExportValue(node, NPGUIKey::ShowAnimData,		mShowAnimData); */
        /* 	NPXmlHelper::ExportValue(node, NPGUIKey::HideAnimData,		mHideAnimData); */
        /* 	NPXmlHelper::ExportValue(node, NPGUIKey::DeadAnimData,		mDeadAnimData); */
        /* 	NPXmlHelper::ExportValue(node, NPGUIKey::DownAnimData,		mDownAnimData); */
        /* 	NPXmlHelper::ExportValue(node, NPGUIKey::RiseAnimData,		mRiseAnimData); */
        /* 	NPXmlHelper::ExportValue(node, NPGUIKey::UserAnimData,		mUserAnimData); */
        ui_data_np_save_bool(s, level + 1, "FireAnimClip", ctx.m_data_anim->fire_anim_clip);
        ui_data_np_save_str(s, level + 1, "ShowSFXFile", ctx.m_data_sound->show_sfx_file);
        ui_data_np_save_str(s, level + 1, "HideSFXFile", ctx.m_data_sound->hide_sfx_file);
        ui_data_np_save_str(s, level + 1, "DnSFXFile", ctx.m_data_sound->down_sfx_file);
        ui_data_np_save_str(s, level + 1, "UpSFXFile", ctx.m_data_sound->rise_sfx_file);
        ui_data_np_save_str(s, level + 1, "PushSFXFile", ctx.m_data_sound->push_sfx_file);
	}

    if (ctx.m_data_window) {
        ui_data_np_save_bool(s, level + 1, "Modal", ctx.m_data_window->mudal);
        ui_data_np_save_str(s, level + 1, "OkayCtrl", "");
        ui_data_np_save_str(s, level + 1, "DenyCtrl", "");
        ui_data_np_save_str(s, level + 1, "HideCtrl", "");
        ui_data_np_save_str(s, level + 1, "HeadCtrl", "");
        ui_data_np_save_str(s, level + 1, "TextCtrl", "");
        ui_data_np_save_bool(s, level + 1, "StartVisible", ctx.m_data_window->start_show);
        ui_data_np_save_str(s, level + 1, "ClassName", "UserWindow");
        ui_data_np_save_bool(s, level + 1, "DialogTemplate", ctx.m_data_window->dialog_template);
    }

    if (rv) return rv;

    /*遍历子控件 */
    ui_data_control_childs(&control_it, control);
    child_control = ui_data_control_it_next(&control_it);
    if (child_control) {
        stream_putc_count(s, ' ', (level + 1) * 4);
        stream_printf(s, "<Children>\n");

        for(; child_control; child_control = ui_data_control_it_next(&control_it)) {
            if (ui_data_np_save_control_i(s, mgr, child_control, level + 2, em) != 0) {
                rv = -1;
            }
        }

        stream_putc_count(s, ' ', (level + 1) * 4);
        stream_printf(s, "</Children>\n");
    }

    stream_putc_count(s, ' ', level * 4);
    stream_printf(s, "</%s>\n", tag_name);

    return rv;
}

static int ui_data_np_save_layout_i(write_stream_t s, ui_data_mgr_t mgr, ui_data_layout_t layout, error_monitor_t em) {
    ui_data_control_t root_control;

    stream_printf(s, "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n");

    root_control = ui_data_layout_root(layout);
    if (root_control == NULL) {
        CPE_ERROR(em, "ui_data_np_save_layout: no root control!");
        return -1;
    }

    if (ui_data_np_save_control_i(s, mgr, root_control, 0, em) != 0) {
        return -1;
    }

    return 0;
}

int ui_data_np_save_layout(void * ctx, ui_data_mgr_t mgr, ui_data_src_t src, const char * root, error_monitor_t em) {
    FILE * fp = NULL;
    int rv = -1;
    struct write_stream_file fs;

    fp = ui_data_np_save_open_file(root, src, em);
    if (fp == NULL) goto COMPLETE;

    write_stream_file_init(&fs, fp, em);
    rv = ui_data_np_save_layout_i((write_stream_t)&fs, mgr, ui_data_src_product(src), em);

COMPLETE:
    if (fp) file_stream_close(fp, em);
    return rv;
}

static void * ui_np_control_data_from_entry(ui_data_mgr_t mgr, ui_data_control_t control, const char * entry_name) {
    LPDRMETA data_meta;
    LPDRMETAENTRY entry;

    data_meta = ui_data_control_meta(mgr, ui_data_control_type(control));
    assert(data_meta);

    entry = dr_meta_find_entry_by_name(data_meta, entry_name);

    if (entry) {
        return ((char *)ui_data_control_data(control)) + dr_entry_data_start_pos(entry, 0);
    }
    else {
        return NULL;
    }
}

static const char * ui_data_np_control_type_name(ui_data_control_type_t control_type) {
    switch (control_type) {
    case ui_data_control_window:
        return "NPGUIWindow";
    default:
        return ui_data_np_control_tag_name(control_type);
    }
}
