product:=conn_net_bpg
$(product).type:=lib
$(product).depends:=conn_net_cli usf_bpg_pkg
$(product).c.sources:=$(wildcard $(product-base)/*.c)

$(eval $(call product-def,$(product)))
