product:=ui_sprite_barrage
$(product).type:=lib
$(product).depends:=ui_sprite ui_sprite_fsm ui_sprite_2d ui_sprite_cfg ui_plugin_barrage
$(product).c.libraries:=
$(product).c.sources:=$(wildcard $(product-base)/*.c)

$(eval $(call product-def,$(product)))
