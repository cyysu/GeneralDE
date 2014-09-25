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
#include "cpe/utils/math_ex.h"
#include "cpe/pal/pal_stdio.h"
#include "cpe/pal/pal_stdlib.h"
#include "cpe/utils/string_utils.h"
#include "gdpp/app/Application.hpp"
#include "gdpp/app/Log.hpp"
#include "uipp/sprite/World.hpp"
#include "uipp/sprite/Entity.hpp"
#include "AnimationFactory.hpp"
#include "RControlNode.hpp"
#include "ContextExt.hpp"
#include "GroupNode.hpp"
#include "uipp/sprite_np/NpUtils.hpp"

namespace UI { namespace Sprite { namespace R {

class AnimationFactoryImpl : public AnimationFactory {
public:
    RNode * createAnimation(
        Gd::App::Application & app, ContextExt * contextExt,
        const char * input_res, const uint8_t is_loop, const int32_t start, const int32_t end)
    {
        size_t res_len = strlen(input_res) + 1;
        ::std::vector<char> res_buf(res_len, 0);
    
        memcpy(&res_buf[0], input_res, res_len);

        if (char * p = strrchr(&res_buf[0], ':')) {
            *p = 0;
            return createByType(app, contextExt, &res_buf[0], p + 1, is_loop, start, end);
        }
        else {
            return createByResName(app, contextExt, &res_buf[0], is_loop, start, end);
        }
    }

private:
    RNode * createByType(
        Gd::App::Application & app, ContextExt * contextExt,
        const char * type_name, char * arg, const uint8_t is_loop, const int32_t start, const int32_t end)
    {
        if(strcmp(type_name, "img-block") == 0) {
            int resId;
            if (!UI::Sprite::R::NpUtils::readResId(arg, resId)) {
                APP_CTX_ERROR(app, "read res %s id fail!", arg);
                return NULL;
            }

            uint32_t guid;
            sscanf(arg, FMT_UINT32_T, &guid);

            int fileID = R2DSImageFileMgr::GetIns()->GetFileID((npu32)guid);
            if (fileID < 0) {
                APP_CTX_ERROR(app, "image %d not exist!", guid);
                return NULL;
            }

            return createImage(fileID, resId);
        }
        else if(strcmp(type_name, "frame") == 0) {
            int resId;
            if (!UI::Sprite::R::NpUtils::readResId(arg, resId)) {
                APP_CTX_ERROR(app, "read res %s id fail!", type_name);
                return NULL;
            }

            uint32_t guid;
            sscanf(arg, FMT_UINT32_T, &guid);

            int fileID = R2DSFrameFileMgr::GetIns()->GetFileID((npu32)guid);
            if (fileID < 0) {
                APP_CTX_ERROR(app, "frame %d not exist!", guid);
                return NULL;
            }

            return createFrame(fileID, resId);
        }
        else if(strcmp(type_name, "actor") == 0) {
            int resId;
            if (!UI::Sprite::R::NpUtils::readResId(arg, resId)) {
                APP_CTX_ERROR(app, "read res id fail(%s)!", arg);
                return NULL;
            }

            uint32_t guid;
            sscanf(arg, FMT_UINT32_T, &guid);

            int fileID = R2DSActorFileMgr::GetIns()->GetFileID((npu32)guid);
            if (fileID < 0) {
                APP_CTX_ERROR(app, "actor file %d not exist!", guid);
                return NULL;
            }

            return createAction(fileID, resId, start);
        }
        else if (strcmp(type_name, "BOX") == 0) {
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

            return new RectNode(lt_x, lt_y, rb_x, rb_y,getColor(color));
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

			return new CircleNode(p_x, p_y,radius,getColor(color));
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

            return new ChainNode(nodes, getColor(color));
		}
		else if (strcmp(type_name, "Template") == 0) {
			 return createControlNode(arg);	
		}
		else if (strcmp(type_name, "ExternCtrl") == 0) {
			return createExternCtrlNode(app, contextExt, arg);
		}
        else {
            APP_CTX_ERROR(app, "unknown type %s!",type_name);
            return NULL;
        }
    }

    RNode * createByResName(
        Gd::App::Application & app, ContextExt * contextExt, 
        char * res, const uint8_t is_loop, const int32_t start, const int32_t end)
    {
        int resId;
        if (!UI::Sprite::R::NpUtils::readResId(res, resId)) {
            APP_CTX_ERROR(app, "read res id fail(%s)!", res);
            return NULL;
        }

        char * file;
        const char * path;

        if (char * p = strrchr(res, '/'))  {
            *p = 0;
            path = res;
            file = p + 1;
        }
        else {
            path = "";
            file = res;
        }

        char * postfix;
        if (char * p = strrchr(file, '.')) {
            postfix = p + 1;
        }
        else {
            APP_CTX_ERROR(app, "get file %s postfix fail!", file);
            return NULL;
        }

        if (strcmp(postfix, "act") == 0) {
            int fileID = R2DSActorFileMgr::GetIns()->GetFileID(path, file);
            if (fileID < 0) {
                APP_CTX_ERROR(app, "actor %s-%s not exist!", path, file);
                return NULL;
            }

            return createAction(fileID, resId, start);
        }
        else if (strcmp(postfix, "frm") == 0) {
            int fileID = R2DSFrameFileMgr::GetIns()->GetFileID(path, file);
            if (fileID < 0) {
                APP_CTX_ERROR(app, "sprite: %s-%s not exist!", path, file);
                return NULL;
            }

            return createFrame(fileID, resId);
        }
        else if (strcmp(postfix, "ibk") == 0) {
            int fileID = R2DSImageFileMgr::GetIns()->GetFileID(path, file);
            if (fileID < 0) {
                APP_CTX_ERROR(app, "module: %s-%s not exist!", path, file);
                return NULL;
            }
            return createImage(fileID, resId);
        }
        else {
            APP_CTX_ERROR(app, "unknown type %s!", postfix);
            return NULL;
        }
    }

	class RectNode :public RNode{
	public:
		RectNode(npf32 lt_x, npf32 lt_y, npf32 rb_x, npf32 rb_y, RColor color) {
			m_lt_x = lt_x;
			m_lt_y = lt_y;
			m_rb_x = rb_x;
			m_rb_y = rb_y;
			m_color = color;
			m_color.a = 0.5f;
		}

		void Render(void) {
			RNode::Render();
			const RVector3 & scale = GetParent()->GetFinalS();
			const RVector3 & trans = GetParent()->GetFinalT();
			npf32 left   =  m_lt_x * scale.x + trans.x;
			npf32 top    =  m_lt_y * scale.y + trans.y;
			npf32 right  =  m_rb_x * scale.x + trans.x;
			npf32 bottom =  m_rb_y * scale.y + trans.y;

			R2DSRenderUtil::GetIns()->DrawRect(RVector2(left,top), RVector2(right,bottom), 
				RMatrix4x4::Identity, m_color );
			R2DSRenderUtil::GetIns()->DrawLineBox(RVector2(left,top), RVector2(right,bottom), 
				RMatrix4x4::Identity,RColor::Red );
		}

	private:
		npf32 m_lt_x;
		npf32 m_lt_y;
		npf32 m_rb_x;
		npf32 m_rb_y;
		//npu32 m_angle;
		RColor m_color;
	};

	class CircleNode :public RNode{
	public:
		CircleNode(npf32 p_x, npf32 p_y, npf32 radius, RColor color) {
			m_p_x = p_x;
			m_p_y = p_y;
			m_radius = radius;
			m_color = color;
			m_color.a = 0.5f;
		}

		void Render(void) {
			RNode::Render();
			const RVector3 & scale = GetParent()->GetFinalS();
			const RVector3 & trans = GetParent()->GetFinalT();
			RVector2 center = RVector2(m_p_x * scale.x + trans.x,  m_p_y * scale.y + trans.y);
			int count = 30;

			for (int i =0; i < count; i++ )
			{
				float x1 = center.x + scale.x * m_radius * cos(i * 2 * M_PI / count);
				float y1 = center.y + scale.x * m_radius * sin(i * 2 * M_PI / count);
				float x2 = center.x + scale.x * m_radius * cos((i+1) * 2 * M_PI / count);
				float y2 = center.y + scale.x * m_radius * sin((i+1) * 2 * M_PI / count);

				RVector2 min = RVector2(x1, y1);
				RVector2 max = RVector2(x2, y2);
				R2DSRenderUtil::GetIns()->DrawLine(min ,max,RMatrix4x4::Identity, RColor::Red);
				
				RVector2 pts[3];
				pts[0] = center;
				pts[1] = min;
				pts[2] = max;
				R2DSRenderUtil::GetIns()->DrawPoly(pts, RMatrix4x4::Identity, m_color);
			}
		}

	private:
		npf32 m_p_x;
		npf32 m_p_y;
		npf32 m_radius;
		RColor m_color;
	};

    class ChainNode : public RNode {
    public:
        ChainNode(::std::vector<RVector2>& nodes, RColor color)
            : m_color(color)
        {
            m_nodes.swap(nodes);
			m_color = RColor::Yellow;
			m_color.a = 0.5f;
        }

		void Render(void) {
			RNode::Render();
			const RVector3 & scale = GetParent()->GetFinalS();
			const RVector3 & trans = GetParent()->GetFinalT();

			std::vector<RVector2>::iterator node_it = m_nodes.begin();
			RVector2 start_pos = RVector2(trans.x + node_it->x * scale.x, trans.y + node_it->y * scale.y);
			
			for(; (node_it+1) != m_nodes.end(); ++node_it)
			{
				float x1 = trans.x + node_it->x * scale.x;
				float y1 = trans.y + node_it->y * scale.y;
				float x2 = trans.x + (node_it+1)->x * scale.x;
				float y2 = trans.y + (node_it+1)->y * scale.y;
				RVector2 min = RVector2(x1, y1);
				RVector2 max = RVector2(x2, y2);
				R2DSRenderUtil::GetIns()->DrawLine(min ,max,RMatrix4x4::Identity, RColor::Red);
				RVector2 pts[3];
				pts[0] = start_pos;
				pts[1] = min;
				pts[2] = max;
				R2DSRenderUtil::GetIns()->DrawPoly(pts, RMatrix4x4::Identity, m_color);
			}
		}

    private:
        ::std::vector<RVector2> m_nodes;
		RColor m_color;
    };

	class PolyNode : public RNode{
	public:
		PolyNode(RVector2* pts, npu32 count, RColor color){

			for (uint8_t i=0; i < 4; i++)
			{
				m_pts[i].x = pts[i].x;
				m_pts[i].y = pts[i].y;
			}
			
			m_color = color;
			m_count = count;
		}

		void Render(void) {
			RNode::Render();
			const RVector3 & scale = GetParent()->GetFinalS();
			RVector2 pts[4];
			for (npu32 i=0; i< m_count; i++)
			{
				pts[i].x = m_pts[i].x * scale.x ;
				pts[i].x = m_pts[i].y * scale.y ;
			}
			R2DSRenderUtil::GetIns()->DrawPoly(pts,
				RMatrix4x4::Identity,RColor::Red);
		}

		RVector2 m_pts[4];
		RColor m_color;
		npu32 m_count;
	};

    RNode * createImage(npu32 fileID, const int32_t id) {
        R2DSImageRef * node = new R2DSImageRef();
        node->SetImage(fileID, id);
        return node;
    }

	RNode * createFrame(npu32 fileID, const int32_t id) {
        R2DSFrameRef * node = new R2DSFrameRef();
		node->SetFrame(fileID, id);
        return node;
	}

	RNode * createControlNode(const char * res ) {
        RControlNode* node = new RControlNode(res);
        return node;
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

    RNode * createAction(npu32 fileID, const int32_t id, int32_t startFrame) {
        R2DSActorRef * node = new R2dActorRefExt();
		node->SetActor(fileID, id);
		node->SetTime(startFrame<0 ? 0 : startFrame);
		node->SetPlay(true);
        return node;
	}

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

	RColor getColor(const char * color)
	{
		if(strcmp(color, "black") == 0){
			return RColor::Black; 
		}
		else if(strcmp(color, "red") == 0){
			return RColor::Red; 
		}
		else if(strcmp(color, "green") == 0){
			return RColor::Green;
		}
		else if(strcmp(color, "blue") == 0){
			return RColor::Blue;
		}
		else if(strcmp(color, "yellow") == 0){
			return RColor::Yellow;
		}
		else if(strcmp(color, "gray") == 0){
			return RColor::Gray;
		}
		else if(strcmp(color, "orange") == 0){
			return RColor::Orange;
		}
		else{
			return RColor::White;
		}
	}
};

AnimationFactory::~AnimationFactory() {
}

AnimationFactory & AnimationFactory::instance(void) {
    static AnimationFactoryImpl s_ins;
    return s_ins;
}

}}}

