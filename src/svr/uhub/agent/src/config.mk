product:=uhub_agent
$(product).type:=lib
$(product).depends:=set_stub
$(product).c.sources:=$(wildcard $(product-base)/*.c)

$(eval $(call product-def,$(product)))
