product:=uipp_sprite_np
$(product).type:=lib
$(product).depends:=cpepp_utils gdpp_app \
                    drow_render \
                    uipp_sprite uipp_sprite_fsm uipp_sprite_2d uipp_sprite_anim 
$(product).c.flags.ld:=
$(product).c.sources:=$(wildcard $(product-base)/*.cpp)

$(eval $(call product-def,$(product)))
