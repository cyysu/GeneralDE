product:=net_trans
$(product).type:=lib
$(product).depends:=libev curl gd_app
$(product).c.libraries:=
$(product).c.sources:=$(wildcard $(product-base)/*.c)

$(eval $(call product-def,$(product)))
