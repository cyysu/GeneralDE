product:=usf_logic_use
$(product).type:=cpe-dr lib
$(product).depends:=cpe_utils usf_logic
$(product).c.flags.ld:=
$(product).c.sources:=$(wildcard $(product-base)/*.c)

$(eval $(call product-def,$(product),tools))
