product:=account_agent
$(product).type:=cpe-dr lib 
$(product).depends:=cpepp_cfg cpepp_dr cpe_dr_data_cfg cpe_dr_data_pbuf \
                    cpepp_dp cpepp_nm gdpp_app gdpp_utils gd_dr_cvt \
                    usfpp_logic_use usfpp_logic usfpp_bpg_pkg usfpp_bpg_rsp \
                    usfpp_mongo_driver usfpp_mongo_cli usfpp_mongo_use \

$(product).c.sources:=$(filter-out %/main.cpp,$(wildcard $(product-base)/*.cpp))

$(product).product.c.includes:=$(subst $(CPDE_ROOT)/,,$(call c-source-dir-to-binary-dir,$(product-base),server))
$(product).product.c.flags.ld:=-rdynamic
$(product).product.c.output-includes:=share

$(product).cpe-dr.modules:=pro
#编译data协议定义
$(product).cpe-dr.pro.generate:=h c
$(product).cpe-dr.pro.source:=$(addprefix $(product-base)/../../svr/pro/, cli/svr_account_pro.xml svr/svr_account_internal.xml)
$(product).cpe-dr.pro.h.output:=share/protocol/svr/account
$(product).cpe-dr.pro.h.with-traits:=pro_meta_traits.cpp
$(product).cpe-dr.pro.c.output:=share/protocol/svr/account/metalib.c
$(product).cpe-dr.pro.c.arg-name:=g_metalib_svr_account_pro

$(eval $(call product-def,$(product)))

