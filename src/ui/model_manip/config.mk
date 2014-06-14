product:=ui_model_manip
$(product).type:=lib 
$(product).depends:=cpe_plist ui_model ui_model_ed
$(product).c.libraries:=
$(product).c.sources:=$(wildcard $(product-base)/*.c)
$(eval $(call product-def,$(product)))

