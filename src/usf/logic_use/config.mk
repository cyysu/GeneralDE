product:=usf_logic_use
$(product).type:=cpe-dr lib
$(product).depends:=cpe_utils usf_logic
$(product).c.flags.ld:=
$(product).c.sources:=$(wildcard $(product-base)/*.c)

$(product).cpe-dr.modules:=asnyc_op_info
$(product).cpe-dr.asnyc_op_info.generate:=h c
$(product).cpe-dr.asnyc_op_info.source:=$(product-base)/logic_op_async_info.xml
$(product).cpe-dr.asnyc_op_info.h.output:=protocol/logic_use
$(product).cpe-dr.asnyc_op_info.c.output:=protocol/logic_use/logic_op_async_info.c
$(product).cpe-dr.asnyc_op_info.c.arg-name:=g_metalib_logic_use

$(eval $(call product-def,$(product),tools))
