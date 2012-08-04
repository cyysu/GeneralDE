product:=usfpp_pom_gs
$(product).type:=lib
$(product).depends:=cpepp_utils gdpp_app usf_pom_gs
$(product).c.flags.ld:=
$(product).c.sources:=$(wildcard $(product-base)/*.cpp)

$(eval $(call product-def,$(product)))
