product:=ui_sprite_spine
$(product).type:=lib
$(product).depends:=ui_sprite ui_sprite_fsm ui_spine
$(product).c.libraries:=
$(product).c.sources:=$(wildcard $(product-base)/*.c)

$(eval $(call product-def,$(product)))
