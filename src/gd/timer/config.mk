product:=gd_timer
$(product).type:=lib
$(product).depends:=cpe_tl gd_app
$(product).c.libraries:=
$(product).c.sources:=$(wildcard $(product-base)/*.c)
$(product).c.flags.cpp:=-DGD_TIMER_DEBUG

$(eval $(call product-def,$(product)))
