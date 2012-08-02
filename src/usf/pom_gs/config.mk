product:=usf_pom_gs
$(product).type:=lib
$(product).depends:=cpe_utils cpe_cfg cpe_dr_data_basic cpe_pom_grp gd_app
$(product).c.flags.ld:=
$(product).c.sources:=$(wildcard $(product-base)/*.c)

$(eval $(call product-def,$(product)))
