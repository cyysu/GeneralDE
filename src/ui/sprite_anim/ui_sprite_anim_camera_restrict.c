#include "ui_sprite_anim_camera_restrict_i.h"

ui_sprite_anim_camera_restrict_t
ui_sprite_anim_camera_restrict_create(
    ui_sprite_anim_camera_t camera,
    const char * name,
    ui_sprite_anim_camera_restrict_fun_t fun,
    void * ctx)
{
    ui_sprite_anim_module_t module = camera->m_module;
    ui_sprite_anim_camera_restrict_t camera_restrict;
    size_t name_len = strlen(name) + 1;

    camera_restrict = mem_alloc(module->m_alloc, sizeof(struct ui_sprite_anim_camera_restrict) + name_len);
    if (camera_restrict == NULL) {
        CPE_ERROR(module->m_em, "camera: add restrict %s: alloc fail", name);
        return NULL;
    }

    memcpy(camera_restrict + 1, name, name_len);

    camera_restrict->m_camera = camera;
    camera_restrict->m_name = (char*)(camera_restrict + 1);
    camera_restrict->m_fun = fun;
    camera_restrict->m_ctx = ctx;

    TAILQ_INSERT_HEAD(&camera->m_restricts, camera_restrict, m_next_for_camera);

    return camera_restrict;
}

ui_sprite_anim_camera_restrict_t
ui_sprite_anim_camera_restrict_find(ui_sprite_anim_camera_t camera, const char * name) {
    ui_sprite_anim_camera_restrict_t camera_restrict;

    TAILQ_FOREACH(camera_restrict, &camera->m_restricts, m_next_for_camera) {
        if (strcmp(camera_restrict->m_name, name) == 0) return camera_restrict;
    }

    return NULL;
}

void ui_sprite_anim_camera_restrict_free(ui_sprite_anim_camera_restrict_t camera_restrict) {
    ui_sprite_anim_camera_t camera = camera_restrict->m_camera;

    TAILQ_REMOVE(&camera->m_restricts, camera_restrict, m_next_for_camera);

    mem_free(camera->m_module->m_alloc, camera_restrict);
}

void ui_sprite_anim_camera_restrict_adj(
    ui_sprite_anim_camera_t camera, UI_SPRITE_2D_PAIR * pos, float * scale)
{
    ui_sprite_anim_camera_restrict_t camera_restrict;

    TAILQ_FOREACH(camera_restrict, &camera->m_restricts, m_next_for_camera) {
        camera_restrict->m_fun(camera_restrict->m_ctx, camera, pos, scale);
    }
}
