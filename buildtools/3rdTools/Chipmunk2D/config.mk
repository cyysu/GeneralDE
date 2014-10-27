product:=Chipmunk2D
$(product).type:=lib
$(product).depends:=
$(product).c.libraries:=
$(product).c.sources:=$(wildcard $(CPDE_ROOT)/3rdTools/Chipmunk2D/src/*.c)
$(product).c.includes:=3rdTools/Chipmunk2D/src 3rdTools/Chipmunk2D/include/chipmunk
$(product).product.c.includes:=3rdTools/Chipmunk2D/include

$(eval $(call product-def,$(product)))
