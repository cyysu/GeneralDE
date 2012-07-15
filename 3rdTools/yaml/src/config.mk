product:=yaml
$(product).type:=lib
$(product).product.c.includes:=3rdTools/yaml/include
$(product).c.sources:=$(wildcard $(product-base)/*.c)
$(product).c.env-includes:=3rdTools/yaml/src
$(product).c.flags.cpp:=-DHAVE_CONFIG_H
$(product).c.flags.lan.c:= -O2 -Wno-parentheses-equality
$(product).c.flags.ld:=

$(eval $(call product-def,$(product)))
