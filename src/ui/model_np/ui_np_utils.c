#include "cpe/pal/pal_string.h"
#include "ui_np_utils.h"

const char * ui_data_np_postfix(ui_data_src_type_t type) {
    switch(type) {
    case ui_data_src_type_dir:
        return "";
    case ui_data_src_type_module:
        return "npModule";
    case ui_data_src_type_sprite:
        return "npSprite";
    case ui_data_src_type_action:
        return "npAction";
    case ui_data_src_type_layout:
        return "npLayout";
    case ui_data_src_type_particle:
        return "particle";
    case ui_data_src_type_texture_png:
        return "png";
    default:
        return NULL;
    }
}

const char * ui_data_np_control_tag_name(ui_data_control_type_t control_type) {
    switch (control_type) {
    case ui_data_control_window:
        return "Window";
    case ui_data_control_panel:
        return "NPGUIPanel";
    case ui_data_control_picture:
        return "NPGUIPicture";
    case ui_data_control_label:
        return "NPGUILabel";
    case ui_data_control_button:
        return "NPGUIButton";
    case ui_data_control_toggle:
        return "NPGUIToggle";
    case ui_data_control_progress:
        return "NPGUIProgressBar";
    case ui_data_control_picture_cond:
        return "NPGUIPictureCondition";
    default:
        return NULL;
    }
}

static struct {
    const char * type_name;
    uint32_t type_hash;
} s_particle_mod_defs[] = {
    /* 1*/ { "NPParticleAttractAccelMOD", 0x2794b9ef },
    /* 2*/ { "NPParticleDampingAccelMOD", 0xfd70fb4d },
    /* 3*/ { "NPParticleSeedAccelMOD", 0xABCDD4B8 },
    /* 4*/ { "NPParticleSineAccelMOD", 0x383f6c77 },
    /* 5*/ { "NPParticleCurvedColorMOD", 0x85fab0a2 },
    /* 6*/ { "NPParticleCurvedAlphaMOD", 0xc82a0e7b },
    /* 7*/ { "NPParticleFixedColorMOD", 0xC1368471 },
    /* 8*/ { "NPParticleOverLifeColorMOD", 0x27453D9B },
    /* 9*/ { "NPParticleSeedColorMOD", 0xFB12331C },
    /*10*/ { "NPParticleSeedLifetimeMOD", 0x14474AFD },
    /*11*/ { "NPParticleOrbitLocationMOD", 0xd4f63e01 },
    /*12*/ { "NPParticleSeedLocationMOD", 0x29BAB14E },
    /*13*/ { "NPParticleSeedRotation2DMOD", 0x61334946 },
    /*14*/ { "NPParticleCurvedUniformSizeMOD", 0x85e88758},
    /*15*/ { "NPParticleCurvedSizeMOD", 0x51916d2f },
    /*16*/ { "NPParticleUniformOverLifeSizeMOD", 0xA36FC378 },
    /*17*/ { "NPParticleOverLifeSizeMOD", 0xd42b5a2a },
    /*18*/ { "NPParticleSeedSizeMOD", 0x8FC2AA86 },
    /*19*/ { "NPParticleUniformSeedSizeMOD", 0x13CB2444 },
    /*20*/ { "NPParticleFlipbookUVMOD", 0x4bb9a385 },
    /*21*/ { "NPParticleScrollAnimMOD", 0x03850cb0 },
    /*22*/ { "NPParticleTileSubTexMOD", 0x67e0030c },
    /*23*/ { "NPParticleCircleSpawnMOD", 0x85849DEA },
    /*24*/ { "NPParticleEllipseSpawnMod", 0xbf3738cf },
    /*25*/ { "NPParticleAttractVelocityMOD", 0xbeeed874 },
    /*26*/ { "NPParticleSeedVelocityMOD", 0xBB55B0F7 },
    /*27*/ { "NPParticleHorizontalStopMOD", 0x6629e4c8 }
};

const char * ui_data_np_particle_mod_type_name(uint8_t mod_type) {
    if (mod_type < 1 || (mod_type - 1) >= CPE_ARRAY_SIZE(s_particle_mod_defs)) {
        return "unknown-mode-type";
    }

    return s_particle_mod_defs[mod_type - 1].type_name;
}

uint32_t ui_data_np_particle_mod_type_hash(uint8_t mod_type) {
    if (mod_type < 1 || (mod_type - 1) >= CPE_ARRAY_SIZE(s_particle_mod_defs)) {
        return 0;
    }

    return s_particle_mod_defs[mod_type - 1].type_hash;
}

uint8_t ui_data_np_particle_mod_type(const char * mod_type_name) {
    uint8_t i;
    for(i = 0; i < CPE_ARRAY_SIZE(s_particle_mod_defs); ++i) {
        if (strcmp(s_particle_mod_defs[i].type_name, mod_type_name) == 0) {
            return i + 1;
        }
    }

    return 0;
}
