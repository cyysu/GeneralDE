#include <assert.h>
#include "RType.h"
#include "RColor.h"
#include "R2DSRenderUtil.h"
#include "R2DSActorRef.h"
#include "R2DSFrameFileMgr.h"
#include "R2DSActorFileMgr.h"
#include "R2DSImageFileMgr.h"
#include "R2DSActor.h"
#include "R2DSSceneFileMgr.h"
#include "R2DSSceneFile.h"
#include "R2DSSceneLayer.h"
#include "R2DSImageRef.h"
#include "R2DSFrameRef.h"
#include "R2DSActorRef.h"
#include "RSkeleton.hpp"
#include "RBarrage.hpp"
#include "RRectNode.h"
#include "RCircleNode.h"
#include "RChainNode.h"
#include "cpe/utils/math_ex.h"
#include "cpe/pal/pal_stdio.h"
#include "cpe/pal/pal_stdlib.h"
#include "cpe/utils/string_utils.h"
#include "gdpp/app/Application.hpp"
#include "gdpp/app/Log.hpp"
#include "render/model/ui_object_ref.h"
#include "render/model/ui_data_mgr.h"
#include "render/model/ui_data_src.h"
#include "render/model/ui_data_spine.h"
#include "render/spine/ui_spine_obj.h"
#include "ui/sprite_spine/ui_sprite_spine_obj.h"
#include "ui/sprite_spine/ui_sprite_spine_env.h"
#include "ui/sprite_barrage/ui_sprite_barrage_env.h"
#include "uipp/sprite/World.hpp"
#include "uipp/sprite/Entity.hpp"
#include "AnimationFactory.hpp"
#include "RControlNode.hpp"
#include "ModuleExt.hpp"
#include "ContextExt.hpp"
#include "GroupNode.hpp"
#include "uipp/sprite_render/NpUtils.hpp"

namespace UI { namespace Sprite { namespace R {

class AnimationFactoryImpl : public AnimationFactory {
public:
    RNode * createAnimation(
        Gd::App::Application & app, ContextExt * contextExt,
        const char * input_res, const uint8_t is_loop, const int32_t start, const int32_t end)
    {
        char buf[256];

        if (const char * sep = strrchr(input_res, ':')) {
            if(cpe_str_cmp_part(input_res, sep - input_res, "Template") == 0) {
                return new RControlNode(sep + 1);
            }
            else if(cpe_str_cmp_part(input_res, sep - input_res, "ExternCtrl") == 0) {
                return createExternCtrlNode(app, contextExt, sep + 1);
            }
            else if(cpe_str_cmp_part(input_res, sep - input_res, "skeleton") == 0) {
                if (strcmp(sep + 1, "self") == 0) {
                    return new RSelfRSkeleton();
                }
            }
            else if(cpe_str_cmp_part(input_res, sep - input_res, "barrage") == 0) {
                if (strcmp(sep + 1, "all") == 0) {
                    ui_sprite_barrage_env_t barrage_env = ui_sprite_barrage_env_find(contextExt->world());
                    if (barrage_env == NULL) {
                        APP_CTX_ERROR(app, "AnimationFactory: create barrage node: barrage env fail!");
                        return NULL;
                    }
                    
                    RBarrage * barrage = new RBarrage();
                    barrage->setBarrage(ui_sprite_barrage_env_env(barrage_env));
                    return barrage;
                }
                else {
                    return NULL;
                }
            }
            else {
                if (node_builder const * builder = contextExt->module().findNodeBuilder(input_res, sep - input_res)) {
                    return builder->m_build_fun(contextExt->world(), builder->m_build_ctx, sep + 1);
                }
            }
        }

        UI_OBJECT_URL * url = ui_object_ref_parse(input_res, buf, sizeof(buf), app.em());
        if (url == NULL) {
            APP_CTX_ERROR(app, "parse url %s fail!", input_res);
            return NULL;
        }

        switch(url->type) {
        case UI_OBJECT_TYPE_IMG_BLOCK: {
            int fid = getFid<R2DSImageFileMgr>(app, input_res, url->data.img_block.src, "ibk");
            if (fid < 0) return NULL;

            R2DSImageRef * node = new R2DSImageRef();
            node->SetImage((uint32_t)fid, url->data.img_block.id);
            return node;
        }
        case UI_OBJECT_TYPE_FRAME: {
            int fid = getFid<R2DSFrameFileMgr>(app, input_res, url->data.frame.src, "frm");
            if (fid < 0) return NULL;

            R2DSFrameRef * node = new R2DSFrameRef();
            node->SetFrame((uint32_t)fid, url->data.frame.id);
            return node;
        }
        case UI_OBJECT_TYPE_ACTOR: {
            int fid = getFid<R2DSActorFileMgr>(app, input_res, url->data.actor.src, "act");
            if (fid < 0) return NULL;

            R2DSActorRef * node = new R2dActorRefExt();
            node->SetActor((uint32_t)fid, url->data.actor.id);
            node->SetTime(0);
            node->SetPlay(true);
            return node;
        }
        case UI_OBJECT_TYPE_PARTICLE:
            return NULL;
        case UI_OBJECT_TYPE_SKELETON: {
            return createSpineNode(app, input_res, contextExt, url->data.skeleton);
        }
        default:
            APP_CTX_ERROR(app, "parse url %s: not support url type %d!", input_res, url->type);
            return NULL;
        }
    }

private:
    template<typename T>
    int getFid(Gd::App::Application & app, const char * input_res, UI_OBJECT_SRC_REF const & src, const char * postfix) { 
        switch(src.type) {
        case UI_OBJECT_SRC_REF_TYPE_BY_ID: {
            int fid = T::GetIns()->GetFileID(src.data.by_id.src_id);
            if (fid < 0) {
                APP_CTX_ERROR(
                    app, "parse url %s: image %d not exist!",
                    input_res, src.data.by_id.src_id);
                return -1;
            }
            return fid;
        }
        case UI_OBJECT_SRC_REF_TYPE_BY_PATH: {
            char buf[128];
            snprintf(buf, sizeof(buf), "%s.%s", (const char *)src.data.by_path.path, postfix);
            int fid = T::GetIns()->GetFileIDFull(buf);
            if (fid < 0) {
                APP_CTX_ERROR(
                    app, "parse url %s: image %s not exist!",
                    input_res, src.data.by_path.path);
                return -1;
            }
            return fid;
        }
        default:
            APP_CTX_ERROR(
                app, "parse url %s: src type %d unknown!",
                input_res, src.type);
            return -1;
        }
    }

    RNode * createSpineNode(Gd::App::Application & app, const char * input_res, ContextExt * contextExt, UI_OBJECT_URL_DATA_SKELETON const & url) {
        ui_sprite_spine_env_t spine_env = ui_sprite_spine_env_find(contextExt->world());
        if (spine_env == NULL) {
            APP_CTX_ERROR(app, "parse url %s: spine env not exist!", input_res);
            return NULL;
        }

        ui_data_mgr_t data_mgr = ui_data_mgr_find_nc(app, NULL);
        if (data_mgr == NULL) {
            APP_CTX_ERROR(app, "parse url %s: ui data_mgr not exist!", input_res);
            return NULL;
        }

        ui_data_src_t src = ui_data_src_child_find_by_path(
            ui_data_mgr_src_root(data_mgr),
            (const char *)url.src.data.by_path.path, ui_data_src_type_spine);
        if (src == NULL) {
            APP_CTX_ERROR(app, "parse url %s: src %s not exist!", input_res, (const char *)url.src.data.by_path.path);
            return NULL;
        }

        ui_data_spine_t spine_data = (ui_data_spine_t )ui_data_src_product(src);
        if (spine_data == NULL) {
            APP_CTX_ERROR(app, "parse url %s: src %s not loaded!", input_res, (const char *)url.src.data.by_path.path);
            return NULL;
        }

        spSkeletonData * skeleton_data = (spSkeletonData *)ui_data_spine_skeleton_data(spine_data);
        if (skeleton_data == NULL) {
            APP_CTX_ERROR(app, "parse url %s: skeleton data %s not loaded!", input_res, (const char *)url.src.data.by_path.path);
            return NULL;
        }

        ui_spine_obj_t spine_obj = ui_spine_obj_create_with_data(ui_sprite_spine_env_obj_mgr(spine_env), skeleton_data, 0);
        if (spine_obj == NULL) {
            APP_CTX_ERROR(app, "parse url %s: create spine obj fail!", input_res);
            return NULL;
        }

        RSkeleton * skeleton = new RSkeleton();
        skeleton->setObj(spine_obj, true);
        return skeleton;
    }

    RNode * createByType(
        Gd::App::Application & app, ContextExt * contextExt,
        const char * type_name, char * arg, const uint8_t is_loop, const int32_t start, const int32_t end)
    {
        if (strcmp(type_name, "BOX") == 0) {
            npf32 lt_x = 0.0f;
            npf32 lt_y = 0.0f;
            npf32 rb_x = 0.0f;
            npf32 rb_y = 0.0f;
			char * color = 0;
            char * name;
            char * value;
            do {
                char * p = strchr(arg, ',');
                if (p) *p = 0;

                if (!readArg(arg, name, value)) {
                    APP_CTX_ERROR(app, "box read arg fail");
                    return NULL;
                }

                if (strcmp(name, "lt.x") == 0) {
                    lt_x = atof(value);
                }
                else if (strcmp(name, "lt.y") == 0) {
                    lt_y = atof(value);
                }
                else if (strcmp(name, "rb.x") == 0) {
                    rb_x = atof(value);
                }
                else if (strcmp(name, "rb.y") == 0) {
                    rb_y = atof(value);
                }
				else if (strcmp(name, "color") == 0) {
                    color = value;
                }
                else {
                    APP_CTX_ERROR(app, "box read unkonwn arg '%s'", name);
                    return NULL;
                }

                if (p) {
                    arg = p + 1;
                }
                else {
                    break;
                }
            } while(1);

            if (rb_x <= lt_x || rb_y <= lt_y) {
                APP_CTX_ERROR(
                    app, "get Box Data error: lt.x=%f, lt.y=%f, rb.x=%f, rb.y=%f",
                    lt_x, lt_y, rb_x, rb_y);
                return NULL;
            }

            return new RRectNode(lt_x, lt_y, rb_x, rb_y, RColor::color(color));
        }
		else if (strcmp(type_name, "CIRCLE") == 0) {
			npf32 p_x = 0.0f;
			npf32 p_y = 0.0f;
			npf32 radius = 0.0f;
			char * color = 0;
			char * name;
			char * value;
			do {
				char * p = strchr(arg, ',');
				if (p) *p = 0;

				if (!readArg(arg, name, value)) {
					APP_CTX_ERROR(app, "box read arg fail");
					return NULL;
				}

				if (strcmp(name, "p.x") == 0) {
					p_x = atof(value);
				}
				else if (strcmp(name, "p.y") == 0) {
					p_y = atof(value);
				}
				else if (strcmp(name, "radius") == 0) {
					radius = atof(value);
				}
				else if (strcmp(name, "color") == 0) {
					color = value;
				}
				else {
					APP_CTX_ERROR(app, "circle read unkonwn arg '%s'", name);
					return NULL;
				}

				if (p) {
					arg = p + 1;
				}
				else {
					break;
				}
			} while(1);

			return new RCircleNode(p_x, p_y,radius, RColor::color(color));
		}
		else if (strcmp(type_name, "CHAIN") == 0) {
            const char * color = "";
			//char * str_color = cpe_str_read_and_remove_arg(arg, "color", ',', '=');
   //         if (str_color == NULL) {
   //             color = 1;
   //         }
   //         else {
   //             color = atoi(str_color);
   //             printf("xxxxxx: color=%d\n");
   //         }

            ::std::vector<RVector2> nodes;
            char * begin;
            while((begin = strchr(arg, '('))) {
                char * sep = strchr(begin, ',');
                if (sep == NULL) break;

                char * end = strchr(sep, ')');
                if (end == NULL) break;

                *sep = 0;
                *end = 0;

                nodes.push_back(RVector2(strtof(begin + 1, NULL), strtof(sep + 1, NULL)));

                arg = end + 1;
            }

            return new RChainNode(nodes, RColor::color(color));
		}
        else {
            APP_CTX_ERROR(app, "unknown type %s!",type_name);
            return NULL;
        }
    }

	RNode * createExternCtrlNode(Gd::App::Application & app, ContextExt * contextExt, const char * res ) {
		RGUIControl * exterCtrl = contextExt->findExternCtrl(res);
		if(exterCtrl == NULL)
		{
			APP_CTX_ERROR(app, "createExternCtrlNode res=%s not exist", res);
			return NULL;
		}

		RControlNode* node = new RControlNode(exterCtrl);
		return node;
	}

    class RSelfRSkeleton : public RSkeleton {
    public:
        void Render(void) {
            if (obj() == NULL) {
                if (!setupObj()) return;
            }

            RSkeleton::Render();
        }

        void Update(npf32 deltaTime) {
            if (obj() == NULL) {
                if (!setupObj()) return;
            }

            RSkeleton::Update(deltaTime);
        }

    private:
        bool setupObj(void) {
            GroupNode * groupNode = dynamic_cast<GroupNode *>(GetParent());

            if (groupNode->entityId() == 0) {
                APP_CTX_ERROR(groupNode->world().app(), "RSelfRSkeleton: setupObj: no entity id");
                return false;
            }

            Entity * targetEntity = groupNode->world().findEntity(groupNode->entityId());
            if (targetEntity == NULL) {
                APP_CTX_ERROR(groupNode->world().app(), "RSelfRSkeleton: setupObj: entity %d not exist", groupNode->entityId());
                return false;
            }

            ui_sprite_spine_obj_t obj = ui_sprite_spine_obj_find(*targetEntity);
            if (obj == NULL) {
                APP_CTX_ERROR(groupNode->world().app(), "RSelfRSkeleton: setupObj: entity %d(%s) no spine obj", groupNode->entityId(), targetEntity->name());
                return false;
            }

            setObj(ui_sprite_spine_obj_data(obj), false);
            return true;
        }
    };

	class R2dActorRefExt : public R2DSActorRef {
	public:
        void FireEvent() {
            if (mFireEventList.empty()) return;

            GroupNode * groupNode = dynamic_cast<GroupNode *>(GetParent());

            if (groupNode->entityId() == 0) {
                for(std::vector<std::string>::iterator event_it = mFireEventList.begin();
                    event_it != mFireEventList.end();
                    ++event_it)
                {
                    printf("xxxx: event %s\n", event_it->c_str());
                }
            }
            else {
                Entity * targetEntity = groupNode->world().findEntity(groupNode->entityId());
                if (targetEntity == NULL) {
                    APP_CTX_ERROR(
                        groupNode->world().app(), "Action FireEvent: entity %d not exist!",
                        groupNode->entityId());
                    return;
                }

                for(std::vector<std::string>::iterator event_it = mFireEventList.begin();
                    event_it != mFireEventList.end();
                    ++event_it)
                {
                    targetEntity->buildAndSendEvent(event_it->c_str());
                }
            }
        }
	};

    static bool readArg(char * input, char * & name, char * & value) {
        char * p = strchr(input, '=');
        if (p == NULL) return false;

        value = cpe_str_trim_head(p + 1);
        *cpe_str_trim_tail(value + strlen(value), value) = 0;

        *p = 0;
        name = cpe_str_trim_head(input);
        *cpe_str_trim_tail(p, name) = 0;

        return true;
    }
};

AnimationFactory::~AnimationFactory() {
}

AnimationFactory & AnimationFactory::instance(void) {
    static AnimationFactoryImpl s_ins;
    return s_ins;
}

}}}

