product:=bz2

$(product).type:=virtual
$(product).product.c.libraries+=bz2

$(eval $(call product-def,$(product)))
