#include "cpe/utils/string_utils.h"
#include "uipp/sprite_np/NpUtils.hpp"
#include "uipp/sprite/Entity.hpp"
#include "uipp/sprite_anim/AnimationSch.hpp"
#include "uipp/sprite_2d/Transform.hpp"
#include "gdpp/app/Log.hpp"
#include "NP2DSImageRef.h"
#include "NP2DSFrameRef.h"
#include "NP2DSActorRef.h"
#include "NP2DSFrameFileMgr.h"
#include "NP2DSActorFileMgr.h"
#include "NP2DSImageFileMgr.h"
#include "NP2DSImageFile.h"
#include "NP2DSFrameFile.h"
#include "NP2DSActorFile.h"
#include "NP2DSImage.h"
#include "NP2DSFrame.h"
#include "NP2DSActor.h"
#include "NP2DSCollider.h"
#include "NP2DSLayer.h"
#include "NP2DSFrameKey.h"
#include "NPRTTI.h"
#include "NPGUIPictureCondition.h"
#include "NPGUIProgressBar.h"
#include "NPGUIPicture.h"

namespace UI { namespace Sprite { namespace NP {

	 uint32_t NpUtils::getFileID(const char * typeName, const char * path, const char * file){
		 if(strcmp(typeName, NP2DSImageRef::sRTTI.GetName()) == 0)
		 {
			 return NP2DSImageFileMgr::GetIns()->GetFileID(path, file);
		 }
		 else if(strcmp(typeName, NP2DSFrameRef::sRTTI.GetName()) == 0)
		 {
			 return NP2DSFrameFileMgr::GetIns()->GetFileID(path, file);
		 }
		 else if(strcmp(typeName, NP2DSActorRef::sRTTI.GetName()) == 0)
		 {
			 return NP2DSActorFileMgr::GetIns()->GetFileID(path, file);
		 }
		 else
		 {
			 APP_THROW_EXCEPTION(::std::runtime_error, "getFileID for %s/%s unknow type typeName =%s", path, file, typeName);
		 }
		 return 0;
	 }

	 uint32_t NpUtils::getFileID(const char * typeName, uint32_t	resFile){
		 if(strcmp(typeName, NP2DSImageRef::sRTTI.GetName()) == 0)
		 {
			 return NP2DSImageFileMgr::GetIns()->GetFileID(resFile);
		 }
		 else if(strcmp(typeName, NP2DSFrameRef::sRTTI.GetName()) == 0)
		 {
			 return NP2DSFrameFileMgr::GetIns()->GetFileID(resFile);
		 }
		 else if(strcmp(typeName, NP2DSActorRef::sRTTI.GetName()) == 0)
		 {
			 return NP2DSActorFileMgr::GetIns()->GetFileID(resFile);
		 }
		 else
		 {
			 APP_THROW_EXCEPTION(::std::runtime_error, "getFileID by resfil=%d unknow type name =%s", resFile, typeName);
		 }
		 return 0;
	 }

	 void NpUtils::loadByType(const char * type_name, char * arg, int resId) {

		 if(strcmp(type_name, NP2DSImageRef::sRTTI.GetName()) == 0) {
			 uint32_t guid;
			 sscanf(arg, FMT_UINT32_T, &guid);

			 int fileID = NP2DSImageFileMgr::GetIns()->GetFileID((npu32)guid);
			 if (fileID < 0) {
				 APP_THROW_EXCEPTION(::std::runtime_error, "image %d not exist!", guid);
				 return;
			 }
			 NP2DSImageFile * file = NP2DSImageFileMgr::GetIns()->GetFile(fileID);
			 file->GetImage(resId);
		 }
		 else if(strcmp(type_name, NP2DSFrameRef::sRTTI.GetName()) == 0) {
			 uint32_t guid;
			 sscanf(arg, FMT_UINT32_T, &guid);

			 int fileID = NP2DSFrameFileMgr::GetIns()->GetFileID((npu32)guid);
			 if (fileID < 0) {
				APP_THROW_EXCEPTION(::std::runtime_error, "frame %d not exist!", guid);
				 return;
			 }
			 NP2DSFrameFile * file = NP2DSFrameFileMgr::GetIns()->GetFile(fileID);
			 file->GetFrame(resId);
		 }
		 else if(strcmp(type_name, NP2DSActorRef::sRTTI.GetName()) == 0) {
			 uint32_t guid;
			 sscanf(arg, FMT_UINT32_T, &guid);

			 int fileID = NP2DSActorFileMgr::GetIns()->GetFileID((npu32)guid);
			 if (fileID < 0) {
				 APP_THROW_EXCEPTION(::std::runtime_error, "actor file %d not exist!", guid);
				 return;
			 }
			 NP2DSActorFile * file = NP2DSActorFileMgr::GetIns()->GetFile(fileID);
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

		 if (strcmp(postfix, "npAction") == 0) {
			 int fileID = NP2DSActorFileMgr::GetIns()->GetFileID(path, file);
			 if (fileID < 0) {
				 APP_THROW_EXCEPTION(::std::runtime_error, "actor %s-%s not exist!", path, file);
				 return;
			 }
			 NP2DSActorFile * file = NP2DSActorFileMgr::GetIns()->GetFile(fileID);
			 for (int i=0; i < file->GetActorCount(); i++)
			 {
				 npu32 id = file->GetActorID(i);
				 NP2DSActor* actor = file->GetActor( id );
				 for (int j=0; j < actor->GetLayerCount(); j++){
					 NP2DSLayer* layer = actor->GetLayer(j);

					 std::list<NP2DSFrameKey*>& frameKeyList = layer->GetFrameKeyList();
					 std::list<NP2DSFrameKey*>::iterator itor = frameKeyList.begin();
					 for (; itor != frameKeyList.end(); ++itor){
						 NP2DSTransRef* ref = (*itor)->GetTransRef();
						 ref->Render();
					 }
				 }
			 }
		 }
		 else if (strcmp(postfix, "npSprite") == 0) {
			 int fileID = NP2DSFrameFileMgr::GetIns()->GetFileID(path, file);
			 if (fileID < 0) {
				 APP_THROW_EXCEPTION(::std::runtime_error, "sprite: %s-%s not exist!", path, file);
				 return;
			 }
			 NP2DSFrameFile * file = NP2DSFrameFileMgr::GetIns()->GetFile(fileID);
			 for (int i=0; i < file->GetFrameCount(); i++){
				npu32 id = file->GetFrameID(i);
				NP2DSFrame* frame =  file->GetFrame(id);
				for (int j=0; j < frame->GetImageRefCount(); j++){
					NP2DSImageRef*	ref = frame->GetImageRef(j);
					ref->Render();
				}
			 }
		 }
		 else if (strcmp(postfix, "npModule") == 0) {
			 int fileID = NP2DSImageFileMgr::GetIns()->GetFileID(path, file);
			 if (fileID < 0) {
				 APP_THROW_EXCEPTION(::std::runtime_error, "module: %s-%s not exist!", path, file);
				 return;
			 }
			 NP2DSImageFile * file = NP2DSImageFileMgr::GetIns()->GetFile(fileID);
			 file->GetImage(resId);
		 }
		 else {
			 APP_THROW_EXCEPTION(::std::runtime_error, "unknown type %s!", postfix);
		 }
	 }

	 void NpUtils::preloadResources(UI::Sprite::Entity & entity) {
		 UI::Sprite::Anim::AnimationSch * anim = entity.findComponent<UI::Sprite::Anim::AnimationSch>();
		 if (anim == NULL) return;

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

	 int NpUtils::getRectNodeList(std::vector<RectNode> & rectNodeVer, char * resName, UI::Sprite::Entity & entity) {
		 const char * typeName = "";
		 uint32_t fileID = 0;
		 uint32_t frameID = 0;
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

				 ::std::vector<char> buff(fileRes, fileRes + strlen(fileRes) + 1);
				 char * res = &buff[0];

				 if (char * p = strrchr(res, '#')) {
					 *p = 0;
					 frameID = atoi(p + 1);
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

				 if (strcmp(postfix, "npModule") == 0) {
					 typeName = "NP2DSImageRef";
				 }
				 else if (strcmp(postfix, "npSprite") == 0) {
					 typeName = "NP2DSFrameRef";
				 }
				 else if (strcmp(postfix, "npAction") == 0) {
					 typeName = "NP2DSActorRef";
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
		 
			 if(strcmp(typeName, NP2DSImageRef::sRTTI.GetName()) == 0)
			 {
				 NP2DSImageFile * file = NP2DSImageFileMgr::GetIns()->GetFile(fileID);
				 NP2DSImage * image = file->GetImage(frameID);
				 UI::Sprite::P2D::Pair lt = {0.0f, 0.0f};
				 UI::Sprite::P2D::Pair rb = {(float)image->GetSrcW(), (float)image->GetSrcH()};
				 RectNode rectNode;
				 rectNode.rect.lt = lt;
				 rectNode.rect.rb = rb;
				 rectNodeVer.push_back(rectNode);
			 }
			 else if(strcmp(typeName, NP2DSFrameRef::sRTTI.GetName()) == 0)
			 {
				 NP2DSFrameFile * file = NP2DSFrameFileMgr::GetIns()->GetFile(fileID);
				 NP2DSFrame * frame = file->GetFrame(frameID);
				 npu16 collCount = frame->GetColliderCount();			
				 for (npu16 i = 0; i< collCount; i++)
				 {
					 NP2DSCollider* collider = frame->GetCollider(i);
					 const NPRect & tempRect= collider->GetRect(); 
					 UI::Sprite::P2D::Pair lt = {(float)tempRect.LT, (float)tempRect.TP};
					 UI::Sprite::P2D::Pair rb = {(float)tempRect.RT, (float)tempRect.BM};
					 RectNode rectNode;
					 rectNode.name = collider->GetName().c_str();
					 rectNode.rect.lt = lt;
					 rectNode.rect.rb = rb;
					 rectNodeVer.push_back(rectNode);
				 }
			 }
			 else if(strcmp(typeName, NP2DSActorRef::sRTTI.GetName()) == 0)
			 {
				 NP2DSActorFile * file = NP2DSActorFileMgr::GetIns()->GetFile(fileID);
				 NP2DSActor * actor = file->GetActor(frameID);
				 NP2DSLayer * layer = actor->GetLayer(0);
				 NP2DSFrameKey* key = layer->GetFrameKey(0);
				 NP2DSTransRef* transRef = key->GetTransRef();
				 if(NP2DSFrameRef* frameRef = dynamic_cast<NP2DSFrameRef*>(transRef))
				 {
					 NP2DSFrame * frame = frameRef->GetFrame();
					 npu16 collCount = frame->GetColliderCount();
					 for (npu16 i = 0; i< collCount; i++)
					 {
						 NP2DSCollider* collider = frame->GetCollider(i);	
						 const NPRect& tempRect = collider->GetRect(); 
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
					 const NPRect & tempRect= transRef->GetLocalBounding();
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
		 std::vector<UI::Sprite::NP::RectNode> rectNodeVer;
		 int r = getRectNodeList(rectNodeVer, resName, entity);
		 assert(r == 0);
		 std::vector<UI::Sprite::NP::RectNode>::iterator itor = rectNodeVer.begin();
		 for (;itor != rectNodeVer.end(); itor++)
		 {
            transform.mergeRect(itor->rect);
		 }
	 }

	 void NpUtils::setBackFrame(NPGUIControl * control, const char * resource) {
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

		 if (strcmp(sep + 1, "npSprite") == 0) {
			 npu32 fileID				= NP2DSFrameFileMgr::GetIns()->GetFileID(path, file,true);
			 if (fileID == -1) {
				 APP_ERROR("set npSprite from %s - %s fail: format error!", path, file);
				 return;
			 }

			 NPGUIFrameRef* frameRef		= new NPGUIFrameRef(fileID, resId);
			 control->SetBackFrame( *frameRef );
			 delete frameRef;
		 }
		 else if (strcmp(sep + 1, "npModule") == 0) {
			 npu32 fileID	= NP2DSImageFileMgr::GetIns()->GetFileID(path, file, true);
			 if (fileID == -1){
				 APP_ERROR("set npModule from %s - %s fail: format error!", path, file);
				 return;
			 }

			 NPGUIFrameRef* frameRef		= new NPGUIFrameRef(fileID, resId);
			 frameRef->restype = RT_IMAGE;
			 control->SetBackFrame( *frameRef );
			 delete frameRef;
		 }
	 }

	int NpUtils::setAttr(NPGUIControl * control, const char * attr, const char * value) {
		if (strcmp(attr, "do-show") == 0) {
			control->Hide();
			control->SetShowAnimPlay();
			return 0;
		}else if (strcmp(attr, "do-hide") == 0) {
			if(control->WasVisible())
				control->SetHideAnimPlay();
			return 0;
		}

		if (NPGUIProgressBar * c = NPDynamicCast(NPGUIProgressBar, control)) {
			if (strcmp(attr, "progress") == 0) {
				c->SetProgress((npf32)atof(value));
				
			}
			else {
				APP_ERROR("NPGUIProgressBar(%s) not support attr %s!", control->GetName().c_str(), attr);
				return -1;
			}
		}
		else if(NPGUIPictureCondition * c = NPDynamicCast(NPGUIPictureCondition, control)) {
			if (strcmp(attr, "index") == 0) {
				c->SetIndex((npu32)atoi(value));
			}
			else {
				APP_ERROR("NPGUIPictureCondition(%s) not support attr %s!", control->GetName().c_str(), attr);
				return -1;
			}
		}
		else if(NPGUIPicture * c = NPDynamicCast(NPGUIPicture, control)) {
			if (strcmp(attr, "back-frame") == 0) {	
				UI::Sprite::NP::NpUtils::setBackFrame(c,value);
			}
			else {
				APP_ERROR("NPGUIPicture(%s) not support attr %s!", control->GetName().c_str(), attr);
				return -1;
			}
		}

		else if (NPGUILabel * c = NPDynamicCast(NPGUILabel, control)) {
			if (strcmp(attr, "text") == 0) {					
				c->SetTextA(value);
			}
			else if (strcmp(attr, "color") == 0) {

			}
			else {
				APP_ERROR("NPGUILabel(%s) not support attr %s!", control->GetName().c_str(), attr);
				return -1;
			}
		}
		else{
			APP_ERROR("NPControl(%s) is not support set attr!", control->GetName().c_str());
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

