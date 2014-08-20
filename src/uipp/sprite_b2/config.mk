product:=uipp_sprite_b2
$(product).type:=cpe-dr lib
$(product).depends:=cpepp_utils gdpp_app \
                    uipp_sprite uipp_sprite_fsm uipp_sprite_2d uipp_sprite_anim \
                    Box2D
$(product).c.flags.ld:=
$(product).c.sources:=$(wildcard $(product-base)/*.cpp)
$(product).product.c.output-includes:=inc

$(product).cpe-dr.modules:=data
$(product).cpe-dr.data.generate:=h c
$(product).cpe-dr.data.source:=$(addprefix $(product-base)/../../ui/pro/sprite_b2/,\
                                   $(shell cat $(product-base)/protocol.def))
$(product).cpe-dr.data.h.output:=inc/protocol/ui/sprite_b2
$(product).cpe-dr.data.h.with-traits:=metalib_ui_sprite_b2_traits.cpp
$(product).cpe-dr.data.c.output:=metalib_ui_sprite_b2.c
$(product).cpe-dr.data.c.arg-name:=g_metalib_ui_sprite_b2

$(eval $(call product-def,$(product)))
