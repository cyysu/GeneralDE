product:=zmq
$(product).type:=lib
$(product).depends:=
$(product).version:=3.2.2
$(product).product.c.includes:=3rdTools/zmq/include
$(product).c.sources := $(wildcard $(product-base)/*.cpp)
$(product).product.c.env-libraries:=
$(product).c.env-libraries:=
$(product).c.env-includes:=3rdTools/zmq/src
$(product).c.flags.cpp:=

$(eval $(call product-def,$(product)))
