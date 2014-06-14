product:=ui_model_tool
$(product).type:=progn 
$(product).depends:=argtable2 yajl cpe_plist cpe_xcalc ui_model ui_model_np ui_model_ed ui_model_manip
$(product).c.libraries:=
$(product).c.sources:=$(wildcard $(product-base)/*.c)
$(eval $(call product-def,$(product),tools))

