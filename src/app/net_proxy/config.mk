product:=app_net_proxy
$(product).type:=cpe-dr lib
$(product).depends:=cpe_net gd_vnet app_net_pkg
$(product).c.flags.ld:=
$(product).c.sources:=$(wildcard $(product-base)/*.c)
$(product).c.include+=$(subst $(CPDE_ROOT),,$(call c-source-dir-to-binary-dir,$(product-base),$3)

$(product).cpe-dr.modules:=base
$(product).cpe-dr.base.generate:=h c
$(product).cpe-dr.base.source:=$(CPDE_ROOT)/protocol/appnet/app_net_protocol.xml
$(product).cpe-dr.base.h.output:=protocol
$(product).cpe-dr.base.c.output:=protocol/data_all.c
$(product).cpe-dr.base.c.arg-name:=g_metalib_app_net

$(eval $(call product-def,$(product)))



