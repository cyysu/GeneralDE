product:=app_net_pkg
$(product).type:=lib
$(product).depends:=cpe_utils cpe_dr_data_json gd_dr_cvt cpe_dr_data_cfg \
                    gd_app gd_dr_store
$(product).c.flags.ld:=
$(product).c.sources:=$(wildcard $(product-base)/*.c)

$(eval $(call product-def,$(product)))
