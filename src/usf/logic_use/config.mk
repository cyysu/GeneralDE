product:=usf_logic_use
$(product).type:=cpe-dr lib
$(product).depends:=cpe_utils usf_logic
$(product).c.flags.ld:=
$(product).c.sources:=$(wildcard $(product-base)/*.c)

$(product).cpe-dr.modules:=op_send_pkg_data
$(product).cpe-dr.op_send_pkg_data.generate:=h c
$(product).cpe-dr.op_send_pkg_data.source:=$(product-base)/logic_op_async_data.xml
$(product).cpe-dr.op_send_pkg_data.h.output:=protocol
$(product).cpe-dr.op_send_pkg_data.c.output:=protocol/op_send_pkg_data_package.c
$(product).cpe-dr.op_send_pkg_data.c.arg-name:=g_metalib_logic_op_async_package

$(eval $(call product-def,$(product),tools))