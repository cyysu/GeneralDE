product:=gd_log
$(product).type:=lib
$(product).depends:=cpe_utils cpe_cfg gd_app log4c
$(product).c.flags.ld:=
$(product).c.sources:=$(wildcard $(product-base)/*.c)

$(eval $(call product-def,$(product)))
