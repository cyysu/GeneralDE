#ifndef UIPP_APP_RUNING_EXT_H
#define UIPP_APP_RUNING_EXT_H
#include "uipp/app/Runing.hpp"
#include "System.hpp"
#include "RFontDraw.h"
#include "RFontInfo.h"

namespace UI { namespace App {
class DebugInfo;
class RuningExt : public Runing {
public:
    RuningExt(EnvExt & env);
    ~RuningExt();

    void init(void);
    void setSize(int32_t w, int32_t h);
    void update(void);
    void doUpdate(float deltaTime);
    void rend(void);

    void pause(void);
    void resume(void);

    void setFps(float fps);

    void processInput(uint16_t _action, uint32_t _id, int16_t _x, int16_t _y, int16_t _oldx, int16_t _oldy);
    bool rendEnable(void) const { return m_rendEnable; }

	virtual float fps(void) const { return m_fps; }
	virtual float runingFps(void) const { return m_runingFps; }

private:
    EnvExt & m_env;
    float m_fps;
    uint32_t m_fixFrameTime;

    int64_t m_curTime;
    int64_t m_lastTime;
    int64_t m_lastUpdateTime;
    int64_t m_offsetRenderTime;
    bool m_rendEnable;
	float m_runingFps;
};

}}

#endif
