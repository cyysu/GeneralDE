product:=Box2D
$(product).type:=lib
$(product).depends:=
$(product).c.sources:=$(shell find $(CPDE_ROOT)/3rdTools/Box2D/Box2D -name *.cpp)
$(product).product.c.includes:=3rdTools/Box2D/Box2D 

$(eval $(call product-def,$(product),))
