product:=apppp_net_cli
$(product).type:=lib
$(product).depends:=cpepp_utils gdpp_app app_net_cli app_net_pkg
$(product).c.flags.ld:=
$(product).c.sources:=$(wildcard $(product-base)/*.cpp)

#$(eval $(call product-def,$(product)))
