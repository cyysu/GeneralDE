product:=app_net_bpg
$(product).type:=lib
$(product).depends:=usf_bpg_pkg app_net_proxy
$(product).c.flags.ld:=
$(product).c.sources:=$(wildcard $(product-base)/*.c)

$(eval $(call product-def,$(product)))



