#ifndef UIPP_APP_RUNING_EXT_H
#define UIPP_APP_RUNING_EXT_H
#include "uipp/app/Runing.hpp"
#include "System.hpp"

namespace UI { namespace App {

class RuningExt : public Runing {
public:
    RuningExt(EnvExt & env);
    ~RuningExt();

    void init(int32_t w, int32_t h);
    void update(void);
    void doUpdate(float deltaTime);
    void rend(void);

    void pause(void);
    void resume(void);

    void processInput(uint16_t _action, uint32_t _id, int16_t _x, int16_t _y, int16_t _oldx, int16_t _oldy);

    bool rendEnable(void) const { return m_rendEnable; }

private:
    EnvExt & m_env;
    uint32_t m_fixFrameTime;

    int64_t m_curTime;
    int64_t m_lastTime;
    int64_t m_lastUpdateTime;
    int64_t m_offsetRenderTime;
    bool m_rendEnable;
};

}}

#endif
