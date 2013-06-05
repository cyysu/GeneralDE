product:=svrpp_conn_cli
$(product).type:=lib
$(product).depends:=cpepp_utils gdpp_app conn_cli
$(product).c.flags.ld:=
$(product).c.sources:=$(wildcard $(product-base)/*.cpp)

$(eval $(call product-def,$(product)))
