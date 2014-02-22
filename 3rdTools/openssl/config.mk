product:=openssl

$(product).type:=virtual
$(product).product.c.includes+=3rdTools/openssl/include
$(product).product.c.defs:=_REENTRANT
$(product).product.c.libraries+=ssl crypto z

$(product).mac.product.c.flags.ld+=-L$(CPDE_ROOT)/3rdTools/openssl/lib/mac
$(product).mac.product.c.libraries+=pthread

$(product).linux32.product.c.flags.ld+=-L$(CPDE_ROOT)/3rdTools/openssl/lib/linux32
$(product).linux32.product.c.libraries+=pthread

$(product).android.product.c.flags.ld+=-L$(CPDE_ROOT)/3rdTools/openssl/lib/android

$(product).version:=openssl-1.0.1f

$(eval $(call product-def,$(product)))
