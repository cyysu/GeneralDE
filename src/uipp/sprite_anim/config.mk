product:=uipp_sprite_anim
$(product).type:=lib
$(product).depends:=uipp_sprite uipp_sprite_fsm \
                    ui_sprite_anim
$(product).c.flags.ld:=
$(product).c.sources:=$(wildcard $(product-base)/*.cpp)

$(eval $(call product-def,$(product)))
