product:=ui_model
$(product).type:=cpe-dr lib
$(product).depends:=cpe_utils cpe_dr cpe_dr_data_basic
$(product).c.libraries:=
$(product).c.sources:=$(wildcard $(product-base)/*.c)
$(product).product.c.output-includes:=inc

$(product).cpe-dr.modules:=data
$(product).cpe-dr.data.generate:=h c
$(product).cpe-dr.data.source:=$(addprefix $(product-base)/../pro/model/,\
                                   $(shell cat $(product-base)/protocol.def))
$(product).cpe-dr.data.h.output:=inc/protocol/ui/model
$(product).cpe-dr.data.c.output:=metalib_ui_model.c
$(product).cpe-dr.data.c.arg-name:=g_metalib_ui_model

$(eval $(call product-def,$(product)))
