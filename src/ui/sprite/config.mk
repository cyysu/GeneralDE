product:=ui_sprite
$(product).type:=lib
$(product).depends:=cpe_utils cpe_xcalc cpe_tl cpe_timer cpe_dr cpe_dr_data_basic \
                    gd_app
$(product).c.libraries:=
$(product).c.sources:=$(wildcard $(product-base)/*.c)
#$(product).product.c.output-includes:=inc

$(eval $(call product-def,$(product)))