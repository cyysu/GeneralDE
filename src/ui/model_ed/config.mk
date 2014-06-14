product:=ui_model_ed
$(product).type:=cpe-dr lib 
$(product).depends:=ui_model cpe_dr_data_json
$(product).c.libraries:=
$(product).c.sources:=$(wildcard $(product-base)/*.c)
$(product).product.c.output-includes:=inc

$(product).cpe-dr.modules:=data
$(product).cpe-dr.data.generate:=h c
$(product).cpe-dr.data.source:=$(addprefix $(product-base)/../pro/model_ed/,\
                                   ui_ed_src.xml \
                                 )
$(product).cpe-dr.data.h.output:=inc/protocol/ui/model_ed
$(product).cpe-dr.data.c.output:=metalib_ui_model_ed.c
$(product).cpe-dr.data.c.arg-name:=g_metalib_ui_model_ed

$(eval $(call product-def,$(product)))

