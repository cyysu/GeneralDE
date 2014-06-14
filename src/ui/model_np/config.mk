product:=ui_model_np
$(product).type:=lib 
$(product).depends:=xml2 ui_model
$(product).c.libraries:=
$(product).c.sources:=$(wildcard $(product-base)/*.c)
$(eval $(call product-def,$(product)))

