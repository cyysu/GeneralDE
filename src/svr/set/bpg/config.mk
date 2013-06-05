product:=set_bpg
$(product).type:=lib
$(product).depends:=set_share usf_bpg_pkg
$(product).c.sources:=$(wildcard $(product-base)/*.c)

$(eval $(call product-def,$(product)))
