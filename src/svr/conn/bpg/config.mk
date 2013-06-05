product:=conn_bpg
$(product).type:=lib
$(product).depends:=conn_cli usf_bpg_pkg
$(product).c.sources:=$(wildcard $(product-base)/*.c)

$(eval $(call product-def,$(product)))
