product:=usf_mongo_cli
$(product).type:=lib
$(product).depends:=usf_mongo_driver usf_logic_use
$(product).product.c.flags.ld:=
$(product).product.c.libraries:=
$(product).c.sources:=$(wildcard $(product-base)/*.c)

$(eval $(call product-def,$(product)))
