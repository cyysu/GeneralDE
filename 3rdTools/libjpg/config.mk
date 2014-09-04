product:=jpg
$(product).type:=lib
#$(product).version:=1.5.7
$(product).product.c.includes:=3rdTools/libjpg/include

$(product).c.sources := $(wildcard $(product-base)/src/*.cpp)
$(product).product.c.libraries:=
$(product).c.flags.cpp:= -DHAVE_CONFIG_H
$(product).c.flags.ld:=

$(eval $(call product-def,$(product)))
