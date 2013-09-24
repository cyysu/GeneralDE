product:=payment_agent
$(product).type:=cpe-dr lib 
$(product).depends:=cpe_dr
$(product).c.sources:=
$(product).product.c.includes:=$(subst $(CPDE_ROOT)/,,$(call c-source-dir-to-binary-dir,$(product-base),server))
$(product).product.c.output-includes:=share

$(product).cpe-dr.modules:=pro
#编译data协议定义
$(product).cpe-dr.pro.generate:=h c
$(product).cpe-dr.pro.source:=$(addprefix $(product-base)/../../svr/pro/, cli/svr_payment_data.xml cli/svr_payment_iap.xml cli/svr_payment_gift.xml cli/svr_payment_pro.xml)
$(product).cpe-dr.pro.h.output:=share/protocol/svr/payment
$(product).cpe-dr.pro.h.with-traits:=pro_meta_traits.cpp
$(product).cpe-dr.pro.c.output:=share/protocol/svr/payment/metalib.c
$(product).cpe-dr.pro.c.arg-name:=g_metalib_svr_payment_pro

$(eval $(call product-def,$(product)))

