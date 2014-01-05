product:=curl
$(product).type:=lib
$(product).c.libraries:=zlib

$(product).c.sources:=$(wildcard $(product-base)/*.c)
$(product).product.c.includes:=3rdTools/curl/include
$(product).product.c.env-includes:=3rdTools/curl/include 3rdTools/curl/include/env/curl
$(product).c.env-includes:=3rdTools/curl/lib
$(product).c.flags.cpp:=-DHAVE_CONFIG_H
$(product).mac.c.flags.warning:=-Wno-deprecated-declarations
$(product).mac.product.c.libraries:=ssl crypto ldap
$(product).linux32.product.c.libraries=ssl crypto idn rt
$(product).linux64.product.c.linux64=ssl crypto idn rt

$(eval $(call product-def,$(product)))
