product:=usfpp_mongo_agent
$(product).type:=lib
$(product).depends:=cpepp_utils gdpp_app usf_mongo_agent
$(product).c.flags.ld:=
$(product).c.sources:=$(wildcard $(product-base)/*.cpp)

$(eval $(call product-def,$(product)))
