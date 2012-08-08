product:=usf_bpg_zip
$(product).type:= lib
$(product).depends:=cpe_utils usf_bpg_pkg zlib
$(product).c.flags.ld:=
$(product).c.sources:=$(wildcard $(product-base)/*.c)
$(eval $(call product-def,$(product),tools))
