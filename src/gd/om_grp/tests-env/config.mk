product:=testenv.gd_om_grp
$(product).type:=lib
$(product).buildfor:=dev
$(product).depends:=testenv.utils testenv.cpe_cfg testenv.cpe_dr 	gd_om_grp
$(product).c.sources:=$(wildcard $(product-base)/*.cpp)

$(eval $(call product-def,$(product)))
