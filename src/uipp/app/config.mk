product:=uipp_app
$(product).type:=cpe-dr lib
$(product).depends:=drow_render gdpp_app gdpp_evt gd_dr_store uipp_sprite uipp_sprite_fsm uipp_sprite_2d uipp_sprite_touch
$(product).c.flags.ld:=
$(product).c.sources:=$(wildcard $(product-base)/*.cpp)
$(product).product.c.output-includes:=inc

$(product).ios.c.sources:=$(wildcard $(product-base)/ios/*.mm) \
                          $(wildcard $(product-base)/ios/*.m) \
                          $(wildcard $(product-base)/ios/SKPSMTPMessage/*.m)

$(product).cpe-dr.modules:=data
$(product).cpe-dr.data.generate:=h c
$(product).cpe-dr.data.source:=$(addprefix $(product-base)/../../ui/pro/app/,\
                                   $(shell cat $(product-base)/protocol.def))
$(product).cpe-dr.data.h.output:=inc/protocol/ui/app
$(product).cpe-dr.data.h.with-traits:=metalib_ui_app_traits.cpp
$(product).cpe-dr.data.c.output:=metalib_ui_app.c
$(product).cpe-dr.data.c.arg-name:=g_metalib_ui_app

$(eval $(call product-def,$(product)))
