#include "RConfig.h"
#include "RRender.h"
#include "RGUIDesktop.h"
#include "R2DSRenderStep.h"
#include "RGUITouchProc.h"
#include "RTouchManager.h"
#include "RStreamingManager.h"
#include "RAudioManager.h"
#include "m3eTypes.h"
#include "cpe/utils/time_utils.h"
#include "uipp/sprite_2d/System.hpp"
#include "uipp/sprite_np/Module.hpp"
#include "uipp/sprite_touch/TouchManager.hpp"
#include "RuningExt.hpp"
#include "EnvExt.hpp"

namespace UI { namespace App {

RuningExt::RuningExt(EnvExt & env)
    : m_env(env)
    , m_fixFrameTime(33)
    , m_curTime(0)
    , m_lastTime(0)
    , m_lastUpdateTime(0)
    , m_offsetRenderTime(0)
    , m_rendEnable(false)
{
}

RuningExt::~RuningExt() {
}

void RuningExt::init(int32_t w, int32_t h) {
	m_lastUpdateTime = m_curTime = m_lastTime = cur_time_ms();

	RConfig::SetGUIDesktop(RVector2((float)w, (float)h));

	RRender::GetIns()->SetResolutionW(w);
	RRender::GetIns()->SetResolutionH(h);
	RRender::GetIns()->SetMatrixProjOrtho2D();

	// int cap = (int)m3eGetDeviceCap();
	// APP_INFO("m3eGetDeviceCap(): %d\n", cap);
    // (void)cap;
	const GLubyte* renderer = glGetString(GL_RENDERER); //返回一个渲染器标识符，通常是个硬件平台
	const GLubyte* version =glGetString(GL_VERSION); //返回当前OpenGL实现的版本号

	char buf_render[256];
	char buf_version[256];
	strncpy(buf_render, (const char*)renderer, 256);
	strncpy(buf_version, (const char*)version, 256);
}

void RuningExt::update(void) {
	m_env.app().tick();

	m_curTime = cur_time_ms();

	int64 diffTime = m_curTime - m_lastTime;
    if (diffTime < 0)  diffTime = 0;

	//android fps limit code
#ifdef ANDROID
	//android framerate limitation
	if (diffTime < m_fixFrameTime)
	{
		NVThreadSleep(m_fixFrameTime - diffTime);

		m_curTime = m3eTimerGetUST();
		diffTime = m_curTime - m_lastTime;
        diffTime = diffTime < 0 ? 0 : (diffTime > m_fixFrameTime ? m_fixFrameTime : diffTime);
	}
#endif

    float deltaTime = diffTime / 1000.f;
    m_lastTime = m_curTime;

#ifdef ANDROID
	//Xiaoxin::AudioManager::Instance()->Update(deltaTime);
#else

	m_offsetRenderTime += diffTime;

    m_rendEnable = m_fixFrameTime <= m_offsetRenderTime;
    if (m_rendEnable) {
        m_offsetRenderTime -= m_fixFrameTime;

        long updateDelta = m_curTime - m_lastUpdateTime;
        m_lastUpdateTime = m_curTime;

        if (updateDelta > m_fixFrameTime) updateDelta = m_fixFrameTime;

        float updateDeltaTime = updateDelta / 1000.f;
        doUpdate(updateDeltaTime);
	}
#endif
}

void RuningExt::doUpdate(float deltaTime) {
    RTouchManager::GetIns()->Update(deltaTime);
    RTouchManager::GetIns()->PostUpdate(deltaTime);
    RGUIDesktop::GetIns()->Update(deltaTime);
    RStreamingManager::GetIns()->UpdateStreamingTask();
    RAudioManager::GetIns()->Update(deltaTime);
}

void RuningExt::rend(void) {
#ifndef ANDROID
	if (!m_rendEnable) {
		return;
    }
#endif

	RRender::GetIns()->Clear(RRender::CLEAR_TARGET, RColor::Black, 1.0f, 0);
    UI::Sprite::R::uipp_sprite_np::instance(m_env.app()).render();
    RGUIDesktop::GetIns()->Render();
	R2DSRenderStep::GetIns()->RenderImmediate();

    //TODO: move to init ?
#ifdef ANDROID
		if(isFirstRender)
		{
			Xiaoxin::SystemUtil::SetInitEndFlag();
			isFirstRender = false;
		}
#endif
}

void RuningExt::processInput(uint16_t _action, uint32_t _id, int16_t _x, int16_t _y, int16_t _oldx, int16_t _oldy) {
    UI::Sprite::P2D::Pair pt;
    pt.x = _x;
    pt.y = _y;

    if (_action == M3E_Touch_MOUSEBEGAN) {
        if(!RGUITouchProc::GetIns()->OnTouchDown(_id, _x, _y, _oldx, _oldy))
            UI::Sprite::Touch::TouchManager::instance(m_env.app()).touchBegin(_id, pt);

#ifdef _APPLE
        // if ( RGUIDesktop::GetIns() && ConfigCenter::Instance()->GetDeviceAdaptor()->GetUsePixelRect().IsPointInside( Vec2Di( _x, _y ) ) == false )
        // {
        //     RGUIDesktop::GetIns()->SetFocusCtrl( NULL );
        // }
#endif
    }
    else if (_action == M3E_Touch_MOUSEMOVED) {
        if(!RGUITouchProc::GetIns()->OnTouchMove(_id, _x, _y,  _oldx, _oldy)) {
            UI::Sprite::Touch::TouchManager::instance(m_env.app()).touchMove(_id, pt);
        }
    }
    else if (_action == M3E_Touch_MOUSEENDED) {
        if(!RGUITouchProc::GetIns()->OnTouchRise(_id, _x, _y,  _oldx, _oldy))
            UI::Sprite::Touch::TouchManager::instance(m_env.app()).touchEnd(_id, pt);
    }

    if (RGUIDesktop::GetIns()) {
        if (RGUIControl* pCtrl = RGUIDesktop::GetIns()->GetFocusCtrl()) {
            if (pCtrl != RGUIDesktop::GetIns()) {
                return;
            }
        }

        if (RGUIDesktop::GetIns()->GetModalTop()) {
            return;
        }
    }

    RTouchManager::GetIns()->OnEvent(_action, _id, _x, _y);
}

void RuningExt::pause(void) {
	// if (Xiaoxin::Game::Instance())
	// {
	//     if ( Xiaoxin::Game::Instance()->IsPaused())
	//         manualPause = true;
	//     else
	//         Xiaoxin::Game::Instance()->Pause();
	// }

	// s_isInterrupt = true;
	//RAudioManager::GetIns()->ReleaseAudioResource();
}

void RuningExt::resume(void) {
// 	//RAudioManager::GetIns()->AcquireAudioResource();
// 	s_isInterrupt = false;
//     lastUpdateTime = lastTime = m3eTimerGetUST();

// #ifdef USE_TERSAFE
//         TssSdkGameStatusInfo game_status;
//         game_status.game_status_ = GAME_STATUS_FRONTEND;
//         game_status.size_ = sizeof(TssSdkGameStatus);
//         tss_sdk_setgamestatus(&game_status);
// #endif
    
//     if ( manualPause )
//     {
//         manualPause = false;
//     }
//     else
//     {
// 		if (Xiaoxin::Game::Instance())
// 		{
// 			Xiaoxin::Game::Instance()->Resume();
// 			Xiaoxin::Game::Instance()->PopUpPauseWnd();
// 		}
//     }

    //Xiaoxin::TPlatformUtil::GetIns()->KillWaitProcess();
}

Runing::~Runing() {
}

}}

