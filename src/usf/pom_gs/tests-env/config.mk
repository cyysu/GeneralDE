product:=testenv.usf_pom_gs
$(product).type:=lib
$(product).buildfor:=dev
$(product).depends:=testenv.cpe_cfg \
                    testenv.gd_app \
                    usf_pom_gs
$(product).c.sources:=$(wildcard $(product-base)/*.cpp)

$(eval $(call product-def,$(product)))
