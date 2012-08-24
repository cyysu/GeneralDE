product:=usf_mongo_agent
$(product).type:=lib
$(product).depends:=bson gd_app cpe_dr
$(product).product.c.flags.ld:=
$(product).product.c.libraries:=
$(product).c.sources:=$(wildcard $(product-base)/*.c)

$(eval $(call product-def,$(product)))
