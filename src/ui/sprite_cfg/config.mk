product:=ui_sprite_cfg
$(product).type:=lib
$(product).depends:=ui_sprite ui_sprite_fsm ui_sprite_basic ui_sprite_anim ui_sprite_touch
$(product).c.libraries:=
$(product).c.sources:=$(wildcard $(product-base)/*.c)

$(eval $(call product-def,$(product)))
