product:=usf_bpg_req
$(product).type:=cpe-dr lib
$(product).depends:=cpe_utils gd_dr_cvt gd_app usf_logic gd_dr_store
$(product).c.flags.ld:=
$(product).c.sources:=$(wildcard $(product-base)/*.c)

$(eval $(call product-def,$(product)))