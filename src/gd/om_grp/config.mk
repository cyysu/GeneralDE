product:=gd_om_grp
$(product).type:=lib
$(product).depends:=cpe_utils cpe_cfg cpe_dr gd_om
$(product).c.flags.ld:=
$(product).c.sources:=$(wildcard $(product-base)/*.c)

$(eval $(call product-def,$(product)))
