#include "cpe/utils/string_utils.h"
#include "uipp/sprite_render/NpUtils.hpp"
#include "uipp/sprite/Entity.hpp"
#include "uipp/sprite_anim/AnimationSch.hpp"
#include "uipp/sprite_2d/Transform.hpp"
#include "ui/sprite_spine/ui_sprite_spine_obj.h"
#include "gdpp/app/Log.hpp"
#include "R2DSImageRef.h"
#include "R2DSFrameRef.h"
#include "R2DSActorRef.h"
#include "R2DSFrameFileMgr.h"
#include "R2DSActorFileMgr.h"
#include "R2DSImageFileMgr.h"
#include "R2DSImageFile.h"
#include "R2DSFrameFile.h"
#include "R2DSActorFile.h"
#include "R2DSImage.h"
#include "R2DSFrame.h"
#include "R2DSActor.h"
#include "R2DSCollider.h"
#include "R2DSLayer.h"
#include "R2DSFrameKey.h"
#include "RRTTI.h"
#include "RGUIPictureCondition.h"
#include "RGUIProgressBar.h"
#include "RGUIPicture.h"

namespace UI { namespace Sprite { namespace R {

	 uint32_t NpUtils::getFileID(const char * typeName, const char * path, const char * file){
		 if(strcmp(typeName, "img-block") == 0)
		 {
			 return R2DSImageFileMgr::GetIns()->GetFileID(path, file);
		 }
		 else if(strcmp(typeName, "frame") == 0)
		 {
			 return R2DSFrameFileMgr::GetIns()->GetFileID(path, file);
		 }
		 else if(strcmp(typeName, "actor") == 0)
		 {
			 return R2DSActorFileMgr::GetIns()->GetFileID(path, file);
		 }
		 else
		 {
			 APP_THROW_EXCEPTION(::std::runtime_error, "getFileID for %s/%s unknow type typeName =%s", path, file, typeName);
		 }
		 return 0;
	 }

	 uint32_t NpUtils::getFileID(const char * typeName, uint32_t	resFile){
		 if(strcmp(typeName, "img-block") == 0)
		 {
			 return R2DSImageFileMgr::GetIns()->GetFileID(resFile);
		 }
		 else if(strcmp(typeName, "frame") == 0)
		 {
			 return R2DSFrameFileMgr::GetIns()->GetFileID(resFile);
		 }
		 else if(strcmp(typeName, "actor") == 0)
		 {
			 return R2DSActorFileMgr::GetIns()->GetFileID(resFile);
		 }
		 else
		 {
			 APP_THROW_EXCEPTION(::std::runtime_error, "getFileID by resfil=%d unknow type name =%s", resFile, typeName);
		 }
		 return 0;
	 }

	 void NpUtils::loadByType(const char * type_name, char * arg, int resId) {

		 if(strcmp(type_name, "img-block") == 0) {
			 uint32_t guid;
			 sscanf(arg, FMT_UINT32_T, &guid);

			 int fileID = R2DSImageFileMgr::GetIns()->GetFileID((npu32)guid);
			 if (fileID < 0) {
				 APP_THROW_EXCEPTION(::std::runtime_error, "image %d not exist!", guid);
				 return;
			 }
			 R2DSImageFile * file = R2DSImageFileMgr::GetIns()->GetFile(fileID);
			 file->GetImage(resId);
		 }
		 else if(strcmp(type_name, "frame") == 0) {
			 uint32_t guid;
			 sscanf(arg, FMT_UINT32_T, &guid);

			 int fileID = R2DSFrameFileMgr::GetIns()->GetFileID((npu32)guid);
			 if (fileID < 0) {
				APP_THROW_EXCEPTION(::std::runtime_error, "frame %d not exist!", guid);
				 return;
			 }
			 R2DSFrameFile * file = R2DSFrameFileMgr::GetIns()->GetFile(fileID);
			 file->GetFrame(resId);
		 }
		 else if(strcmp(type_name, "actor") == 0) {
			 uint32_t guid;
			 sscanf(arg, FMT_UINT32_T, &guid);

			 int fileID = R2DSActorFileMgr::GetIns()->GetFileID((npu32)guid);
			 if (fileID < 0) {
				 APP_THROW_EXCEPTION(::std::runtime_error, "actor file %d not exist!", guid);
				 return;
			 }
			 R2DSActorFile * file = R2DSActorFileMgr::GetIns()->GetFile(fileID);
			 file->GetActor(resId);
		 }
		 else {
			 APP_THROW_EXCEPTION(::std::runtime_error, "unknown type %s!",type_name);
		 }
	 }

	 void NpUtils::loadByResName(char * res, int resId) {
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
			 APP_THROW_EXCEPTION(::std::runtime_error, "get file %s postfix fail!", file);
			 return;
		 }
		 if (strcmp(postfix, "act") == 0) {
			 int fileID = R2DSActorFileMgr::GetIns()->GetFileID(path, file);
			 if (fileID < 0) {
				 APP_THROW_EXCEPTION(::std::runtime_error, "actor %s-%s not exist!", path, file);
				 return;
			 }
			 R2DSActorFile * file = R2DSActorFileMgr::GetIns()->GetFile(fileID);
			 for (int i=0; i < file->GetActorCount(); i++)
			 {
				 npu32 id = file->GetActorID(i);
				 R2DSActor* actor = file->GetActor( id );
				 for (int j=0; j < actor->GetLayerCount(); j++){
					 R2DSLayer* layer = actor->GetLayer(j);

					 std::list<R2DSFrameKey*>& frameKeyList = layer->GetFrameKeyList();
					 std::list<R2DSFrameKey*>::iterator itor = frameKeyList.begin();
					 for (; itor != frameKeyList.end(); ++itor){
						 R2DSTransRef* ref = (*itor)->GetTransRef();
						 ref->Render();
					 }
				 }
			 }
		 }
		 else if (strcmp(postfix, "frm") == 0) {
			 int fileID = R2DSFrameFileMgr::GetIns()->GetFileID(path, file);
			 if (fileID < 0) {
				 APP_THROW_EXCEPTION(::std::runtime_error, "sprite: %s-%s not exist!", path, file);
				 return;
			 }
			 R2DSFrameFile * file = R2DSFrameFileMgr::GetIns()->GetFile(fileID);
			 for (int i=0; i < file->GetFrameCount(); i++){
				npu32 id = file->GetFrameID(i);
				R2DSFrame* frame =  file->GetFrame(id);
				for (int j=0; j < frame->GetImageRefCount(); j++){
					R2DSImageRef*	ref = frame->GetImageRef(j);
					ref->Render();
				}
			 }
		 }
		 else if (strcmp(postfix, "ibk") == 0) {
			 int fileID = R2DSImageFileMgr::GetIns()->GetFileID(path, file);
			 if (fileID < 0) {
				 APP_THROW_EXCEPTION(::std::runtime_error, "module: %s-%s not exist!", path, file);
				 return;
			 }
			 R2DSImageFile * file = R2DSImageFileMgr::GetIns()->GetFile(fileID);
			 file->GetImage(resId);
		 }
		 else {
			 APP_THROW_EXCEPTION(::std::runtime_error, "unknown type %s!", postfix);
		 }
	 }

	 void NpUtils::preloadResources(UI::Sprite::Entity & entity) {
		 if (UI::Sprite::Anim::AnimationSch * anim = entity.findComponent<UI::Sprite::Anim::AnimationSch>()) {
             UI::Sprite::Anim::AnimationDefIterator animationDefIt = anim->animationDefs();
             while(UI::Sprite::Anim::AnimationDef const * animDef = animationDefIt.next()) {
                 const char * fileRes = animDef->animRes();
                 ::std::vector<char> buff(fileRes, fileRes + strlen(fileRes) + 1);
                 char * res = &buff[0];
                 int resId;
                 if (char * p = strrchr(res, '#')) {
                     *p = 0;
                     resId = atoi(p + 1);
                 }

                 if (char * p = strrchr(res, ':'))  {
                     *p = 0;
                     loadByType(res, p+1, resId);
                 }
                 else{
                     loadByResName(res,resId);
                 }
             }
         }
	 }

	 int NpUtils::getRectNodeList(std::vector<RectNode> & rectNodeVer, char * resName, UI::Sprite::Entity & entity) {
		 const char * typeName = "";
		 uint32_t fileID = 0;
		 uint32_t frameID = 0;
         const char * frameName = NULL;
         ::std::vector<char> buff;

		 if (char * p = strchr(resName, ':')) {
				uint32_t resFile =0;
				 resName = p + 1;
				 if (char * p = strrchr(resName, '#')) {
					 *p = 0;
					 frameID = atoi(p + 1);
				 }
				 if (char * p = strrchr(resName, '#')) {
					 *p = 0;
					 resFile = atoi(p + 1);
					 typeName = resName;
				 }
				 fileID = getFileID(typeName, resFile);
			 }
			 else {
				 UI::Sprite::Anim::AnimationSch & anim = entity.component<UI::Sprite::Anim::AnimationSch>();

				 const char * fileRes = NULL;

				 if (resName[0] == '@') {
					 UI::Sprite::Anim::AnimationDef const * animDef = anim.findAnimationDef(resName + 1);
					 assert(animDef);
					 if(animDef == NULL) {	
						 APP_ERROR("get file res error resName = %s", resName);
						 return -1;
					 }
					 fileRes = animDef->animRes();
				 }
				 else {
					 fileRes = resName;
				 }

				 buff.assign(fileRes, fileRes + strlen(fileRes) + 1);
				 char * res = &buff[0];
				 if (char * p = strrchr(res, '#')) {
                     char * endp = NULL;
                     *p = 0;
                     frameID = strtod(p + 1, &endp);
                     if (*endp != 0) {
                         frameName = p + 1;
                         frameID = -1;
                     }
				 }
				 else {
					 APP_THROW_EXCEPTION(::std::runtime_error, "get frame id %s fail!", res);
					 return -1;
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
					 APP_THROW_EXCEPTION(::std::runtime_error, "get file %s postfix fail!", file);
					 return -1;
				 }

				 if (strcmp(postfix, "ibk") == 0) {
					 typeName = "R2DSImageRef";
				 }
				 else if (strcmp(postfix, "frm") == 0) {
					 typeName = "R2DSFrameRef";
				 }
				 else if (strcmp(postfix, "act") == 0) {
					 typeName = "R2DSActorRef";
				 }
				 else {
					 APP_THROW_EXCEPTION(::std::runtime_error, "unknown type %s!", postfix);
					 return -1;
				 }

				 fileID = getFileID(typeName, path, file);
				 if (fileID == -1) {
					 APP_THROW_EXCEPTION(::std::runtime_error,
						 "addCollider: %s%s not exist", path, file);
				 }
			 }
		 
			 if(strcmp(typeName, "img-block") == 0)
			 {
				 R2DSImageFile * file = R2DSImageFileMgr::GetIns()->GetFile(fileID);
				 R2DSImage * image = frameName ? file->GetImage(frameName) : file->GetImage(frameID);
				 UI::Sprite::P2D::Pair lt = {0.0f, 0.0f};
				 UI::Sprite::P2D::Pair rb = {(float)image->GetSrcW(), (float)image->GetSrcH()};
				 RectNode rectNode;
				 rectNode.rect.lt = lt;
				 rectNode.rect.rb = rb;
				 rectNodeVer.push_back(rectNode);
			 }
			 else if(strcmp(typeName, "frame") == 0)
			 {
				 R2DSFrameFile * file = R2DSFrameFileMgr::GetIns()->GetFile(fileID);
                 R2DSFrame * frame = frameName ? file->GetFrame(frameName) : file->GetFrame(frameID);
				 npu16 collCount = frame->GetColliderCount();			
				 for (npu16 i = 0; i< collCount; i++)
				 {
					 R2DSCollider* collider = frame->GetCollider(i);
					 const RRect & tempRect= collider->GetRect(); 
					 UI::Sprite::P2D::Pair lt = {(float)tempRect.LT, (float)tempRect.TP};
					 UI::Sprite::P2D::Pair rb = {(float)tempRect.RT, (float)tempRect.BM};
					 RectNode rectNode;
					 rectNode.name = collider->GetName().c_str();
					 rectNode.rect.lt = lt;
					 rectNode.rect.rb = rb;
					 rectNodeVer.push_back(rectNode);
				 }
			 }
			 else if(strcmp(typeName, "actor") == 0)
			 {
				 R2DSActorFile * file = R2DSActorFileMgr::GetIns()->GetFile(fileID);
                 R2DSActor * actor = frameName ? file->GetActor(frameName) : file->GetActor(frameID);
				 R2DSLayer * layer = actor->GetLayer(0);
				 R2DSFrameKey* key = layer->GetFrameKey(0);
				 R2DSTransRef* transRef = key->GetTransRef();
				 if(R2DSFrameRef* frameRef = dynamic_cast<R2DSFrameRef*>(transRef))
				 {
					 R2DSFrame * frame = frameRef->GetFrame();

					 npu16 collCount = frame->GetColliderCount();
					 for (npu16 i = 0; i< collCount; i++)
					 {
						 R2DSCollider* collider = frame->GetCollider(i);	
						 const RRect& tempRect = collider->GetRect(); 
						 UI::Sprite::P2D::Pair lt = {tempRect.LT + transRef->GetWorldTrans().x, tempRect.TP + transRef->GetWorldTrans().y};
						 UI::Sprite::P2D::Pair rb = {tempRect.RT + transRef->GetWorldTrans().x, tempRect.BM + transRef->GetWorldTrans().y};
						 RectNode rectNode;
						 rectNode.name = collider->GetName().c_str();
						 rectNode.rect.lt = lt;
						 rectNode.rect.rb = rb;
						 rectNodeVer.push_back(rectNode);
					 }
				 }
				 else
				 {   
					 const RRect & tempRect= transRef->GetLocalBounding();
					 UI::Sprite::P2D::Pair lt = {tempRect.LT + transRef->GetWorldTrans().x, tempRect.TP + transRef->GetWorldTrans().y};
					 UI::Sprite::P2D::Pair rb = {tempRect.RT + transRef->GetWorldTrans().x, tempRect.BM + transRef->GetWorldTrans().y};
					 RectNode rectNode;
					 rectNode.rect.lt = lt;
					 rectNode.rect.rb = rb;
					 rectNodeVer.push_back(rectNode);
				 }

			 }else
			 {
				 APP_THROW_EXCEPTION(::std::runtime_error, "unknow type name =%s", typeName);
				 return -1;
			 }
			 return 0;
	 }

	 void NpUtils::setMerge(char * resName, UI::Sprite::World & world, UI::Sprite::Entity & entity){
		 UI::Sprite::P2D::Transform & transform = entity.component<UI::Sprite::P2D::Transform>();
		 std::vector<UI::Sprite::R::RectNode> rectNodeVer;
		 int r = getRectNodeList(rectNodeVer, resName, entity);
		 assert(r == 0);
		 std::vector<UI::Sprite::R::RectNode>::iterator itor = rectNodeVer.begin();
		 for (;itor != rectNodeVer.end(); itor++)
		 {
            transform.mergeRect(itor->rect);
		 }
	 }

	 void NpUtils::setBackFrame(RGUIControl * control, const char * resource) {
		 size_t len = strlen(resource) + 1;
		 ::std::vector<char> buf(len, 0);
		 char * res = &buf[0];
		 memcpy(res, resource, len);

		 int resId;
		 if (!readResId(res, resId)) {
			 return;
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

		 const char * sep = strchr(file, '.');
		 if (sep == NULL) {
			 APP_ERROR("get pic from %s - %s fail: format error!", path, file);
			 return;
		 }
         if (strcmp(sep + 1, "act") == 0) {
			 npu32 fileID				= R2DSActorFileMgr::GetIns()->GetFileID(path, file,true);
			 if (fileID == -1) {
				 APP_ERROR("set act from %s - %s fail: format error!", path, file);
				 return;
			 }
             R2DSActorFile*	file		= R2DSActorFileMgr::GetIns()->GetFile(fileID);
             npu32 actorID				= file->GetActorID(resId);
             RGUIActorRef* actorRef = new RGUIActorRef(fileID , actorID , RGUIEVENT_SHOW, true);
             control->Hide();
             control->DelFireAnimList();
             control->AddFireAnim(actorRef);
             control->Show();
         }
         else if (strcmp(sep + 1, "frm") == 0) {
			 npu32 fileID				= R2DSFrameFileMgr::GetIns()->GetFileID(path, file,true);
			 if (fileID == -1) {
				 APP_ERROR("set frm from %s - %s fail: format error!", path, file);
				 return;
			 }

			 RGUIFrameRef* frameRef		= new RGUIFrameRef(fileID, resId);
			 control->SetBackFrame( *frameRef );
			 delete frameRef;
		 }
		 else if (strcmp(sep + 1, "ibk") == 0) {
			 npu32 fileID	= R2DSImageFileMgr::GetIns()->GetFileID(path, file, true);
			 if (fileID == -1){
				 APP_ERROR("set ibk from %s - %s fail: format error!", path, file);
				 return;
			 }

			 RGUIFrameRef* frameRef		= new RGUIFrameRef(fileID, resId);
			 frameRef->restype = RT_IMAGE;
			 control->SetBackFrame( *frameRef );
			 delete frameRef;
		 }
	 }

	int NpUtils::setAttr(RGUIControl * control, const char * attr, const char * value) {
		if (strcmp(attr, "do-show") == 0) {
			control->Hide();
			control->SetShowAnimPlay();
			return 0;
		}else if (strcmp(attr, "do-hide") == 0) {
			if(control->WasVisible())
				control->SetHideAnimPlay();
			return 0;
		}

		if (RGUIProgressBar * c = RDynamicCast(RGUIProgressBar, control)) {
			if (strcmp(attr, "progress") == 0) {
				c->SetProgress((npf32)atof(value));
				
			}
			else {
				APP_ERROR("RGUIProgressBar(%s) not support attr %s!", control->GetName().c_str(), attr);
				return -1;
			}
		}
		else if(RGUIPictureCondition * c = RDynamicCast(RGUIPictureCondition, control)) {
			if (strcmp(attr, "index") == 0) {
				c->SetIndex((npu32)atoi(value));
			}
			else {
				APP_ERROR("RGUIPictureCondition(%s) not support attr %s!", control->GetName().c_str(), attr);
				return -1;
			}
		}
		else if(RGUIPicture * c = RDynamicCast(RGUIPicture, control)) {
			if (strcmp(attr, "back-frame") == 0) {	
				UI::Sprite::R::NpUtils::setBackFrame(c,value);
			}
			else {
				APP_ERROR("RGUIPicture(%s) not support attr %s!", control->GetName().c_str(), attr);
				return -1;
			}
		}

		else if (RGUILabel * c = RDynamicCast(RGUILabel, control)) {
			if (strcmp(attr, "text") == 0) {					
				c->SetTextA(value);
			}
			else if (strcmp(attr, "color") == 0) {

			}
			else {
				APP_ERROR("RGUILabel(%s) not support attr %s!", control->GetName().c_str(), attr);
				return -1;
			}
		}
		else{
			APP_ERROR("RControl(%s) is not support set attr!", control->GetName().c_str());
			return -1;
		}

		return 0;
	}

	bool NpUtils::readResId(char * res, int & id) {
		 if (char * p = strrchr(res, '#')) {
			 *p = 0;
			 id = atoi(p + 1);
			 return true;
		 }
		 else {
			 return false;
		 }
	 }
}}}

