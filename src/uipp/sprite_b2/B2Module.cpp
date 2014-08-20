#include "cpepp/nm/Object.hpp"
#include "cpepp/nm/Manager.hpp"
#include "gdpp/app/Application.hpp"
#include "gdpp/app/Log.hpp"
#include "gdpp/app/Module.hpp"
#include "gdpp/app/ModuleDef.hpp"
#include "uipp/sprite/Repository.hpp"
#include "uipp/sprite_fsm/Repository.hpp"
#include "uipp/sprite_2d/Transform.hpp"
#include "uipp/sprite_cfg/CfgLoaderExternGen.hpp"
#include "B2WorldExt.hpp"
#include "B2ObjectExt.hpp"
#include "B2ObjectPartExt.hpp"
#include "B2Action_RunTo.hpp"
#include "B2Action_Manipulator.hpp"
#include "B2Action_Suspend.hpp"
#include "B2Action_Setup.hpp"
#include "B2Action_WaitStop.hpp"
#include "B2Action_OnCollision.hpp"
#include "B2Action_WaitCollision.hpp"
#include "B2Action_WaitNotCollision.hpp"

extern "C" char g_metalib_ui_sprite_b2[];

namespace UI { namespace Sprite { namespace B2 {

class uipp_sprite_b2
    : public Cpe::Nm::Object
    , public Cfg::CfgLoaderExternGen<uipp_sprite_b2>
{
public:
    uipp_sprite_b2(Gd::App::Application & app, Gd::App::Module & module, Cpe::Cfg::Node & cfg)
        : m_app(app)
    {
        addResourceLoader(&uipp_sprite_b2::initB2World);
        addComponentLoader(&uipp_sprite_b2::initB2Object);
        addActionLoader(&uipp_sprite_b2::initB2ActionRunTo);
		addActionLoader(&uipp_sprite_b2::initB2ActionManipulator);
		addActionLoader(&uipp_sprite_b2::initB2ActionSuspend);
		addActionLoader(&uipp_sprite_b2::initB2ActionSetup);
		addActionLoader(&uipp_sprite_b2::initB2ActionWaitStop);
		addActionLoader(&uipp_sprite_b2::initB2ActionOnCollision);
		addActionLoader(&uipp_sprite_b2::initB2ActionWaitCollision);
		addActionLoader(&uipp_sprite_b2::initB2ActionWaitNotCollision);

        Repository & repo = Repository::instance(app);
        Fsm::Repository & fsm_repo = Fsm::Repository::instance(app);

        repo.registerEventsByPrefix((LPDRMETALIB)g_metalib_ui_sprite_b2, "ui_sprite_evt_");

        B2ObjectExt::install(repo);
        B2Action_RunTo::install(fsm_repo);
		B2Action_Manipulator::install(fsm_repo);
		B2Action_Suspend::install(fsm_repo);
		B2Action_Setup::install(fsm_repo);
		B2Action_WaitStop::install(fsm_repo);
		B2Action_OnCollision::install(fsm_repo);
		B2Action_WaitCollision::install(fsm_repo);
		B2Action_WaitNotCollision::install(fsm_repo);
    }

    ~uipp_sprite_b2() {
        try {
            Repository & repo = Repository::instance(m_app);
            Fsm::Repository & fsm_repo = Fsm::Repository::instance(m_app);

            repo.unregisterEventsByPrefix((LPDRMETALIB)g_metalib_ui_sprite_b2, "ui_sprite_evt_");

            repo.removeComponentMeta<B2Object>();
            fsm_repo.removeActionMeta<B2Action_RunTo>();
			fsm_repo.removeActionMeta<B2Action_Manipulator>();
			fsm_repo.removeActionMeta<B2Action_Suspend>();
			fsm_repo.removeActionMeta<B2Action_Setup>();
			fsm_repo.removeActionMeta<B2Action_WaitStop>();
			fsm_repo.removeActionMeta<B2Action_OnCollision>();
			fsm_repo.removeActionMeta<B2Action_WaitCollision>();
			fsm_repo.removeActionMeta<B2Action_WaitNotCollision>();
        }
        catch(...) {
        }
    }

    virtual Gd::App::Application & app(void)  {
        return m_app;
    }

    virtual Gd::App::Application const & app(void) const {
        return m_app;
    }

    static cpe_hash_string_t NAME;

private:
    void initB2World(B2WorldExt & world, Cpe::Cfg::Node const & cfg) const {

        world.setDebug(cfg["debug"].dft(0));

        if (Cpe::Cfg::Node const * updatePriority = cfg.findChild("update-priority")) {
            static_cast<B2World&>(world).setUpdatorPriority(updatePriority->asInt8());
        }

        P2D::Pair gravity;
        gravity.x = cfg["gravity.x"].dft(10.0f);
        gravity.y = cfg["gravity.y"].dft(0.0f);
        world.setGravity(gravity);

		world.setPtmRatio(cfg["ptm-ratio"].dft(1.0f));
        world.setStepDuration(cfg["step-duration"].dft(world.stepDuration()));

		world.setDebugLayer(cfg["debug-layer"].dft(""));

        Cpe::Cfg::Node const & boundary = cfg["boundary"];
        if (boundary.isValid()) {
            P2D::Pair lt;
            P2D::Pair br;
            lt.x = boundary["lt.x"].dft(0.0f);
            lt.y = boundary["lt.y"].dft(0.0f);
            br.x = boundary["br.x"].dft(960.0f);
            br.y = boundary["br.y"].dft(640.0f);

            world.setBoundary(lt, br);
        }

		Cpe::Cfg::NodeConstIterator categories = cfg["categories"].childs();
		while(Cpe::Cfg::Node const * check = categories.next()) {
            const char * objType = check->asString(NULL);
            if (objType == NULL) {
				APP_CTX_THROW_EXCEPTION(
                    app(), ::std::runtime_error,
                    "B2World init: categories config format error");
            }

            if (objType[0] == 0) {
				APP_CTX_THROW_EXCEPTION(
                    app(), ::std::runtime_error,
                    "B2World init: categories name is ''");
            }

            world.addObjType(objType);
        }

        world.setGroundMasks(buildObjTypeMask(world, cfg["ground"]));
    }

    void initB2Object(B2ObjectExt & obj, Cpe::Cfg::Node const & cfg) const {
        if (Cpe::Cfg::Node const * mass_node = cfg.findChild("mass")) {
            obj.setMass(mass_node->dft(obj.mass()));
        }

        if (const char * objType = cfg["type"].asString(NULL)) {
            obj.setTypeAndMode(getBodyType(objType), obj.runingMode());
        }

        if (const char * runingMode = cfg["runing-mode"].asString(NULL)) {
            obj.setTypeAndMode(obj.type(), getRuningMode(runingMode));
        }

        if (Cpe::Cfg::Node const * fixed_rotation_node = cfg.findChild("fixed-rotation")) {
            obj.setFixedRotation(fixed_rotation_node->dft(obj.fixedRotation()));
        }

        if (Cpe::Cfg::Node const * bullet_node = cfg.findChild("is-bullet")) {
            obj.setBullet(bullet_node->dft(obj.bullet()));
        }

        if (Cpe::Cfg::Node const * gravity_scale = cfg.findChild("gravity-scale")) {
            obj.setGravityScale(gravity_scale->dft(obj.gravityScale()));
        }

        B2WorldExt & world = obj.world().res<B2WorldExt>();

        if (Cpe::Cfg::Node const * categories_node = cfg.findChild("categories")) {
            obj.setCategories(buildObjTypeMask(world, *categories_node));
        }

        if (Cpe::Cfg::Node const * collisions_node = cfg.findChild("collisions")) {
            obj.setCollisions(buildObjTypeMask(world, *collisions_node));
        }

        Cpe::Cfg::Node const & fixture_node = cfg["fixture"];
        if (fixture_node.isValid()) {
            if (fixture_node.type() == CPE_CFG_TYPE_STRUCT) {
                obj.setDefaultFixtureMeta(
                    buildObjTypeMask(world, fixture_node["categories"]),
                    buildObjTypeMask(world, fixture_node["collisions"]),
                    fixture_node["friction"].dft(0.0f),
                    fixture_node["restitution"].dft(0.0f),
                    fixture_node["density"].dft(0.0f),
                    fixture_node["is-sensor"].dft(0));
            }
            else if (fixture_node.type() == CPE_CFG_TYPE_SEQUENCE) {
                Cpe::Cfg::NodeConstIterator fixtures = fixture_node.childs();
                while(Cpe::Cfg::Node const * child = fixtures.next()) {
                    Cpe::Cfg::Node const & node = child->onlyChild();
                    if (!node.isValid()) {
                        APP_CTX_THROW_EXCEPTION(m_app, ::std::runtime_error, "initB2Object: fixture format error 2!");
                    }

                    obj.addFixtureMeta(
                        node.name().c_str(),
                        buildObjTypeMask(world, node["categories"]),
                        buildObjTypeMask(world, node["collisions"]),
                        node["friction"].dft(0.0f),
                        node["restitution"].dft(0.0f),
                        node["density"].dft(0.0f),
                        node["is-sensor"].dft(0));
                }
                
            }
            else {
                APP_CTX_THROW_EXCEPTION(m_app, ::std::runtime_error, "initB2Object: fixture format error 3!");
            }
        }

        /**/
        Cpe::Cfg::NodeConstIterator parts = cfg["parts"].childs();
        while(Cpe::Cfg::Node const * part_cfg = parts.next()) {
            const char * str_shape = (*part_cfg)["shape"].asString(NULL);

            if (str_shape == NULL) {
                APP_CTX_THROW_EXCEPTION(m_app, ::std::runtime_error, "initB2Object: part shap not configured!");
            }

            B2ObjectPart & part = obj.addPart((*part_cfg)["name"].dft(""));

            UI_SPRITE_B2_SHAPE shape;
            if (strcmp(str_shape, "box") == 0) {
				shape.type = UI_SPRITE_B2_SHAPE_BOX;
				shape.data.box.lt.x = (*part_cfg)["lt.x"].dft(0.0f);
				shape.data.box.lt.y = (*part_cfg)["lt.y"].dft(0.0f);
				shape.data.box.rb.x = (*part_cfg)["rb.x"].dft(0.0f);
				shape.data.box.rb.y = (*part_cfg)["rb.y"].dft(0.0f);
            }
            else if (strcmp(str_shape, "circle") == 0) {
                shape.type = UI_SPRITE_B2_SHAPE_CIRCLE;
				if (const char * center_base = (*part_cfg)["center.base"].asString(NULL)) {
					shape.data.circle.center_base = ui_sprite_2d_transform_pos_policy_from_str(center_base);
					if (shape.data.circle.center_base == 0) {
						APP_CTX_THROW_EXCEPTION(m_app, ::std::runtime_error, "initB2Object: center policy %s unknown!", center_base);
					}
				}
				else {
					shape.data.circle.center_base = UI_SPRITE_2D_TRANSFORM_POS_ORIGIN;
				}

				shape.data.circle.center.x = (*part_cfg)["center.x"].dft(0.0f);
				shape.data.circle.center.y = (*part_cfg)["center.y"].dft(0.0f);
				shape.data.circle.radius = (*part_cfg)["radius"].dft(0.0f);
            }
            else if (strcmp(str_shape, "entity-rect") == 0) {
                shape.type = UI_SPRITE_B2_SHAPE_ENTITY_RECT;
                shape.data.entity_rect.adj.x = (*part_cfg)["adj.x"].dft(0.0f);
                shape.data.entity_rect.adj.y = (*part_cfg)["adj.y"].dft(0.0f);
            }
			else if (strcmp(str_shape, "sector") == 0) {
				shape.type = UI_SPRITE_B2_SHAPE_SECTOR;
				if (const char * center_base = (*part_cfg)["center.base"].asString(NULL)) {
					shape.data.sector.center_base = ui_sprite_2d_transform_pos_policy_from_str(center_base);
					if (shape.data.sector.center_base == 0) {
						APP_CTX_THROW_EXCEPTION(m_app, ::std::runtime_error, "initB2Object: center policy %s unknown!", center_base);
					}
				}
				else {
					shape.data.sector.center_base = UI_SPRITE_2D_TRANSFORM_POS_ORIGIN;
				}

				shape.data.sector.center.x = (*part_cfg)["center.x"].dft(0.0f);
				shape.data.sector.center.y = (*part_cfg)["center.y"].dft(0.0f);
				shape.data.sector.radius = (*part_cfg)["radius"].dft(0.0f);
				shape.data.sector.angle_start = (*part_cfg)["angle-start"].dft(0.0f);
				shape.data.sector.angle_range = (*part_cfg)["angle_range"].dft(0.0f);
				shape.data.sector.angle_step = (*part_cfg)["angle_step"].dft(3.0f);
			}
			else if (strcmp(str_shape, "chain") == 0) {
				shape.type = UI_SPRITE_B2_SHAPE_CHAIN;
				if (const char * center_base = (*part_cfg)["center.base"].asString(NULL)) {
					shape.data.circle.center_base = ui_sprite_2d_transform_pos_policy_from_str(center_base);
					if (shape.data.circle.center_base == 0) {
						APP_CTX_THROW_EXCEPTION(m_app, ::std::runtime_error, "initB2Object: center policy %s unknown!", center_base);
					}
				}
				else {
					shape.data.circle.center_base = UI_SPRITE_2D_TRANSFORM_POS_ORIGIN;
				}

			}
            else {
                APP_CTX_THROW_EXCEPTION(m_app, ::std::runtime_error, "initB2Object: not support shap %s!", str_shape);
            }

            part.createShape(shape);
        }
    }

    void initB2ActionRunTo(B2Action_RunTo & obj, Cpe::Cfg::Node const & cfg) const {
        if (Cpe::Cfg::Node const * ground_node = cfg.findChild("grounds")) {
            obj.setGroundMasks(buildObjTypeMask(obj.world().res<B2WorldExt>(), *ground_node));
        }
    }

	void initB2ActionManipulator(B2Action_Manipulator & obj, Cpe::Cfg::Node const & cfg) const {
	}

	void initB2ActionSuspend(B2Action_Suspend & obj, Cpe::Cfg::Node const & cfg) const {
        uint8_t init_result = obj.resume() ? 1 : 0;
        uint8_t resume = cfg["resume"].dft(init_result);
        obj.setResume(resume ? true : false);

        if (const char * objType = cfg["obj-type"].asString(NULL)) {
            obj.setObjType(getBodyType(objType));
        }

        if (const char * runingMode = cfg["obj-runing-mode"].asString(NULL)) {
            obj.setRuningMode(getRuningMode(runingMode));
        }
	}

	void initB2ActionSetup(B2Action_Setup & obj, Cpe::Cfg::Node const & cfg) const {
		Cpe::Cfg::NodeConstIterator setups = cfg["setups"].childs();
		while(Cpe::Cfg::Node const * child = setups.next()) {
            const char * attr_name = (*child)["attr"].asString(NULL);
            const char * part = (*child)["part"].asString(NULL);
            float value = (*child)["value"].dft(0.0f);

            if (attr_name == NULL) {
                APP_CTX_THROW_EXCEPTION(m_app, ::std::runtime_error, "initB2ActionSetup: attr name not configured!");
            }

            if (strcmp(attr_name, "fixed-rotation") == 0) {
                obj.addSetup(B2Action_Setup::AttrType_FixedRotation, value);
            }
            else if (strcmp(attr_name, "bullet") == 0) {
                obj.addSetup(B2Action_Setup::AttrType_Bullet, value);
            }
            else if (strcmp(attr_name, "gravity-scale") == 0) {
                obj.addSetup(B2Action_Setup::AttrType_GravityScale, value);
            }
            else if (strcmp(attr_name, "mass") == 0) {
                obj.addSetup(B2Action_Setup::AttrType_Mass, value);
            }
            else if (strcmp(attr_name, "friction") == 0) {
                if (part == NULL) {
                    APP_CTX_THROW_EXCEPTION(m_app, ::std::runtime_error, "initB2ActionSetup: attr friction part not configured!");
                }

                obj.addSetup(B2Action_Setup::AttrType_Friction, value, part);
            }
            else if (strcmp(attr_name, "restitution") == 0) {
                if (part == NULL) {
                    APP_CTX_THROW_EXCEPTION(m_app, ::std::runtime_error, "initB2ActionSetup: attr restitution part not configured!");
                }

                obj.addSetup(B2Action_Setup::AttrType_Restitution, value, part);
            }
            else {
                APP_CTX_THROW_EXCEPTION(
                    m_app, ::std::runtime_error,
                    "initB2ActionSetup: attr name %s is unknown, supported attrs: "
                    "[fixed-rotation, bullet, gravity-scale, mass, friction, restitution]!", attr_name);
            }
        }
	}

	void initB2ActionWaitStop(B2Action_WaitStop & obj, Cpe::Cfg::Node const & cfg) const {
	}

	void initB2ActionOnCollision(B2Action_OnCollision & obj, Cpe::Cfg::Node const & cfg) const {
        setupCollision(obj.collision(), obj.world().res<B2WorldExt>(), cfg);
        obj.setOnCollisionBegin(cfg["on-begin"].dft(""));
        obj.setOnCollisionEnd(cfg["on-end"].dft(""));
	}

	void initB2ActionWaitCollision(B2Action_WaitCollision & obj, Cpe::Cfg::Node const & cfg) const {
        setupCollision(obj.collision(), obj.world().res<B2WorldExt>(), cfg);
	}

	void initB2ActionWaitNotCollision(B2Action_WaitNotCollision & obj, Cpe::Cfg::Node const & cfg) const {
        obj.setCategories(buildObjTypeMask(obj.world().res<B2WorldExt>(), cfg["collisions"]));
	}

    void setupCollision(B2Collision & obj, B2WorldExt & world, Cpe::Cfg::Node const & cfg) const {
		Cpe::Cfg::NodeConstIterator parts = cfg["parts"].childs();
		while(Cpe::Cfg::Node const * child = parts.next()) {
            const char * partName = child->asString(NULL);
            if (partName == NULL) {
                APP_CTX_THROW_EXCEPTION(m_app, ::std::runtime_error, "initB2ActionOnCollision: part format error!");
            }

            obj.addPart(partName);
        }

		Cpe::Cfg::NodeConstIterator collisions = cfg["collisions"].childs();
		while(Cpe::Cfg::Node const * child = collisions.next()) {
            const char * objType = child->asString(NULL);
            if (objType == NULL) {
                APP_CTX_THROW_EXCEPTION(m_app, ::std::runtime_error, "initB2ActionOnCollision: collisions format error!");
            }

            obj.addCollision(world, objType);
        }
    }

    uint16_t buildObjTypeMask(B2WorldExt & world, Cpe::Cfg::Node const & node) const {
        uint16_t mask = 0;

		Cpe::Cfg::NodeConstIterator categories = node.childs();
		while(Cpe::Cfg::Node const * child = categories.next()) {
            const char * categoryName = child->asString(NULL);
            if (categoryName == NULL) {
                APP_CTX_THROW_EXCEPTION(m_app, ::std::runtime_error, "category format error!");
            }

            if (strcmp(categoryName, "*") == 0) return 0xFFFF;

            mask |= world.obyType(categoryName);
        }

        return mask;
    }

    ObjectType getBodyType(const char * objType) const {
        if (strcmp(objType, "static") == 0) {
            return OBJECTTYPE_STATIC;
        }
        else if (strcmp(objType, "kinematic") == 0) {
            return OBJECTTYPE_KINEMATIC;
        }
        else if (strcmp(objType, "dynamic") == 0) {
            return OBJECTTYPE_DYNAMIC;
        }
        else {
            APP_CTX_THROW_EXCEPTION(
                app(), ::std::runtime_error,
                "unknown type %s", objType);
        }
    }

    RuningMode getRuningMode(const char * runingMode) const {
        if (strcmp(runingMode, "active") == 0) {
            return RUNINGMODE_ACTIVE;
        }
        else if (strcmp(runingMode, "passive") == 0) {
            return RUNINGMODE_PASSIVE;
        }
        else {
            APP_CTX_THROW_EXCEPTION(
                app(), ::std::runtime_error,
                "unknown runing-mode %s", runingMode);
        }
    }

    Gd::App::Application & m_app;
};

GDPP_APP_MODULE_DEF(uipp_sprite_b2, uipp_sprite_b2);

}}}
