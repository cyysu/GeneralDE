product:=chat_svr_lib
$(product).type:=cpe-dr lib 
$(product).depends:=cpepp_cfg cpepp_dr cpe_dr_data_cfg cpe_dr_data_pbuf \
                    cpepp_dp cpepp_nm cpepp_pom_grp gd_net cpepp_pom gdpp_app gdpp_timer gdpp_utils gd_dr_cvt \
                    usfpp_logic_use usfpp_logic usfpp_bpg_pkg usfpp_bpg_rsp usfpp_bpg_cli

$(product).c.sources:=$(filter-out %/main.cpp,$(wildcard $(product-base)/*.cpp))

$(product).product.c.includes:=$(subst $(CPDE_ROOT)/,,$(call c-source-dir-to-binary-dir,$(product-base)))
$(product).product.c.flags.ld:=-rdynamic

$(product).cpe-dr.modules:=pro
#编译data协议定义
$(product).cpe-dr.pro.generate:=h c
$(product).cpe-dr.pro.source:=$(addprefix $(product-base)/../pro/, cli/svr_chat_pro.xml meta/chanel_info.xml svr/svr_chat_internal_data.xml)
$(product).cpe-dr.pro.h.output:=protocol/svr/chat
$(product).cpe-dr.pro.h.with-traits:=pro_meta_traits.cpp
$(product).cpe-dr.pro.c.output:=protocol/svr/chat/metalib.c
$(product).cpe-dr.pro.c.arg-name:=g_metalib_svr_chat_pro

$(eval $(call product-def,$(product)))

product:=chat_svr
$(product).type:=progn
$(product).depends:=chat_svr_lib usf_bpg_net usf_bpg_rsp usf_bpg_bind argtable2
$(product).c.sources:=$(product-base)/main.cpp
$(product).c.flags.ld:=-lm -rdynamic
$(product).c.export-symbols:=$(patsubst _%,%,$(shell cat $(product-base)/symbols.def))

$(eval $(call product-def,$(product)))

.DELETE_ON_ERROR: $(product-base)/symbols.vc.def
$(product-base)/symbols.vc.def: $(product-base)/symbols.def
	$(CPDE_PERL) $(CPDE_ROOT)/buildtools/tools/gen-vc-symbols.pl --input-file $< --output-file $@

chat_svr: $(product-base)/symbols.vc.def
