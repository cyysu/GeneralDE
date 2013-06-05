product:=usf_mongo_driver
$(product).type:=lib
$(product).depends:=bson gd_app cpe_dr cpe_dr_data_bson cpe_fsm
$(product).product.c.flags.ld:=
$(product).product.c.libraries:=
$(product).c.sources:=$(wildcard $(product-base)/*.c)

$(eval $(call product-def,$(product)))
