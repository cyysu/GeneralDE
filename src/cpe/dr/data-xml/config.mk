product:=cpe_dr_data_xml
$(product).type:=lib
$(product).depends:=cpe_dr cpe_utils xml2
$(product).c.sources:=$(wildcard $(product-base)/*.c)
$(product).c.flags.cpp:=$(if $(filter 64,$(cpe-dr-metalib)),-D CPE_DR_METALIB_SIZE=64)

$(eval $(call product-def,$(product)))
