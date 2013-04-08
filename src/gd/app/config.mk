product:=gd_app
$(product).type:=lib
$(product).depends:=cpe_utils cpe_cfg cpe_tl cpe_dp cpe_nm cpe_net
$(product).android.depends:=cpe_android
$(product).product.c.env-libraries:=dl
$(product).c.flags.ld:=
$(product).c.sources:=$(wildcard $(product-base)/*.c)
$(product).product.c.libraries:=$($(product).c.libraries)
$(product).product.c.flags.ld:=$($(product).c.flags.ld)

gd-app-multi-thread?=1
ifeq ($(gd-app-multi-thread),1)
$(product).c.flags.cpp+=$(if $(filter 1,$(gd-app-multi-thread)), -DGD_APP_MULTI_THREAD )
$(product).product.c.libraries+=pthread
endif

$(eval $(call product-def,$(product)))
