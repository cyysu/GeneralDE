#include "RAudioManager.h"
#include "gdpp/app/Log.hpp"
#include "ui/sprite/ui_sprite_entity_attr.h"
#include "uipp/sprite_fsm/ActionReg.hpp"
#include "UIAction_AudioBGM.hpp"

namespace UI { namespace App {

UIAction_AudioBGM::UIAction_AudioBGM(Sprite::Fsm::Action & action)
	: ActionBase(action)
	, m_res("")
	, m_loop(0)
	, m_Audio_id(-1)
{
}

UIAction_AudioBGM::UIAction_AudioBGM(Sprite::Fsm::Action & action, UIAction_AudioBGM const & o)
	: ActionBase(action)
	, m_res(o.m_res)
	, m_loop(o.m_loop)
	, m_Audio_id(o.m_Audio_id)
{
}

int UIAction_AudioBGM::enter(void) {
	RAudioManager*	audioManager = RAudioManager::GetIns();
	assert(audioManager);
	m_Audio_id = audioManager->AddBGM(m_res);
	assert(m_Audio_id != -1);
	audioManager->StopBGM();
	audioManager->PlayBGM(m_Audio_id, m_loop);
	return 0;
}

void UIAction_AudioBGM::exit(void) {
}

void UIAction_AudioBGM::install(Sprite::Fsm::Repository & repo) {
	Sprite::Fsm::ActionReg<UIAction_AudioBGM>(repo)
		.on_enter(&UIAction_AudioBGM::enter)
		.on_exit(&UIAction_AudioBGM::exit)
		;
}

const char * UIAction_AudioBGM::NAME = "play-bgm";

}}

