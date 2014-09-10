#include "RAudioManager.h"
#include "gdpp/app/Log.hpp"
#include "ui/sprite/ui_sprite_entity_attr.h"
#include "uipp/sprite_fsm/ActionReg.hpp"
#include "UIAction_AudioSFX.hpp"

namespace UI { namespace App {

	UIAction_AudioSFX::UIAction_AudioSFX(Sprite::Fsm::Action & action)
		: ActionBase(action)
		, m_res("")
		, m_Audio_id(-1)
	{
	}

	UIAction_AudioSFX::UIAction_AudioSFX(Sprite::Fsm::Action & action, UIAction_AudioSFX const & o)
		: ActionBase(action)
		, m_res(o.m_res)
		, m_Audio_id(o.m_Audio_id)
	{
	}

	int UIAction_AudioSFX::enter(void) {
		RAudioManager*	audioManager = RAudioManager::GetIns();
		assert(audioManager);
		m_Audio_id = audioManager->AddSFX( m_res );
		assert(m_Audio_id != -1);
		audioManager->StopSFX(m_Audio_id);
		audioManager->PlaySFX(m_Audio_id);
		return 0;
	}

	void UIAction_AudioSFX::exit(void) {

	}

	void UIAction_AudioSFX::install(Sprite::Fsm::Repository & repo) {
		Sprite::Fsm::ActionReg<UIAction_AudioSFX>(repo)
			.on_enter(&UIAction_AudioSFX::enter)
			.on_exit(&UIAction_AudioSFX::exit)
			;
	}

	const char * UIAction_AudioSFX::NAME = "play-sfx";

}}

