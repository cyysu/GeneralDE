#ifndef UIPP_APP_ENV_H
#define UIPP_APP_ENV_H
#include "cpepp/nm/Object.hpp"
#include "gdpp/app/Application.hpp"
#include "System.hpp"
#include "uipp/sprite_2d/System.hpp"

namespace UI { namespace App {

class Env : public Cpe::Nm::Object {
public:
    virtual uint8_t debug(void) const = 0;

    virtual Language language(void) const = 0;
    virtual const char * appName(void) const = 0;

    virtual Gd::App::Application & app(void) = 0;
    virtual Gd::App::Application const & app(void) const = 0;

    virtual UICenter & uiCenter(void) = 0;
    virtual UICenter const & uiCenter(void) const = 0;

    virtual Device & device(void) = 0;
    virtual Device const & device(void) const = 0;

    virtual Sprite::World & world(void) = 0;
    virtual Sprite::World const & world(void) const = 0;

	virtual Sprite::P2D::Pair const & screenSize(void) const = 0;
	virtual Sprite::P2D::Pair const & screenBaseSize(void) const = 0;

	virtual Runing & runing(void) = 0;
	virtual Runing const & runing(void) const = 0;

    virtual const char * visiableMsg(uint32_t msg_id) const = 0;
    virtual const char * visiableMsg(uint32_t msg_id, char * args) const = 0;

    virtual const char * deviceId(void) const = 0;
    virtual const char * documentPath(void) const = 0;

    virtual ~Env();

    static Env & instance(Gd::App::Application & app);
    static cpe_hash_string_t NAME;
};

}}

#endif
