product:=usf_logic_ls
$(product).type:= lib
$(product).depends:=cpe_utils usf_logic
$(product).c.flags.ld:=
$(product).c.sources:=$(wildcard $(product-base)/*.c)


$(eval $(call product-def,$(product),tools))
