product:=cpe_cfg
$(product).type:=lib
$(product).depends:=cpe_utils cpe_dr cpe_zip yaml
$(product).libraries:=
$(product).c.sources:=$(wildcard $(product-base)/*.c)

$(eval $(call product-def,$(product)))
