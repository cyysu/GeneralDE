product:=opengl
$(product).type:=virtual

$(product).mingw.product.c.includes:=3rdTools/gles/mingw/include
$(product).mingw.product.c.libraries:=opengl32

$(eval $(call product-def,$(product)))
