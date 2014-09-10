product:=testenv.ui_sprite_anim
$(product).type:=lib
$(product).buildfor:=dev
$(product).depends:=testenv.utils testenv.cpe_cfg testenv.ui_sprite_fsm ui_sprite_anim
$(product).c.sources:=$(wildcard $(product-base)/*.cpp)

$(eval $(call product-def,$(product)))
