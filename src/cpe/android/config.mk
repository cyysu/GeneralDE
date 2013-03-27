product:=cpe_android
$(product).type:=lib
$(product).depends:=utils
$(product).product.c.includes:=include
$(product).libraries:=
$(product).c.sources:=$(whildcard $(product-base)/*.c)
$(eval $(call product-def,$(product)))
