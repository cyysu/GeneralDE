product:=conn_logic
$(product).type:=lib
$(product).depends:=conn_cli usf_logic
$(product).c.sources:=$(wildcard $(product-base)/*.c)

$(eval $(call product-def,$(product)))
