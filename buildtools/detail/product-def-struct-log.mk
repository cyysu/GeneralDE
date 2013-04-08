product-support-types+=struct-log
product-def-all-items+=struct-log.modules

struct-log-tool=$(CPDE_ROOT)/buildtools/tools/gen-struct-log.pl

define product-def-rule-struct-log-module

$(call assert-not-null,$1.struct-log.$3.source)
$(call assert-not-null,$1.struct-log.$3.output-h)
$(call assert-not-null,$1.struct-log.$3.output-c)
$(call assert-not-null,$1.struct-log.$3.prefix)
$(call assert-not-null,$1.struct-log.$3.categories)

$(eval r.$1.$2.struct-log.$3.source:=$($1.struct-log.$3.source))
$(eval r.$1.$2.struct-log.$3.output-h:=$($1.struct-log.$3.output-h))
$(eval r.$1.$2.struct-log.$3.output-c:=$($1.struct-log.$3.output-c))
$(eval r.$1.$2.struct-log.$3.prefix:=$($1.struct-log.$3.prefix))
$(eval r.$1.$2.struct-log.$3.categories:=$($1.struct-log.$3.categories))
$(eval r.$1.$2.struct-log.$3.generated.h:=$(call c-source-dir-to-binary-dir,$(r.$1.base)/$($1.struct-log.$3.output-h),$2))
$(eval r.$1.$2.struct-log.$3.generated.c:=$(call c-source-dir-to-binary-dir,$(r.$1.base)/$($1.struct-log.$3.output-c),$2))

$(eval r.$1.$2.c.sources += $(r.$1.$2.struct-log.$3.generated.c) $(r.$1.$2.struct-log.$3.generated.h))
$(eval r.$1.cleanup += $(r.$1.$2.struct-log.$3.generated.c) $(r.$1.$2.struct-log.$3.generated.h))

auto-build-dirs += $(dir $(r.$1.$2.struct-log.$3.generated.c) $(r.$1.$2.struct-log.$3.generated.h))

$(call c-source-to-object,$(r.$1.c.sources),$2): $(r.$1.$2.struct-log.$3.generated.h)

$(r.$1.$2.struct-log.$3.generated.c) $(r.$1.$2.struct-log.$3.generated.h): $$(r.$1.$2.struct-log.$3.source) $(struct-log-tool)
	$$(call with_message,struct-log generaing to $(subst $(CPDE_ROOT)/,,$(r.$1.$3.struct-log.$2.generated.c)) ...) \
	LD_LIBRARY_PATH=$(CPDE_OUTPUT_ROOT)/$(tools.output)/lib:$$$$LD_LIBRARY_PATH \
	$(CPDE_PERL) $(struct-log-tool) --input-file $(r.$1.$2.struct-log.$3.source) \
                       --output-c $(r.$1.$2.struct-log.$3.generated.c) \
                       --output-h $(r.$1.$2.struct-log.$3.generated.h) \
                       --prefix $(r.$1.$2.struct-log.$3.prefix) \
                       $(addprefix --category , $(r.$1.$2.struct-log.$3.categories))

endef

define product-def-rule-struct-log

$(foreach module,$(r.$1.struct-log.modules),\
	$(call product-def-rule-struct-log-module,$1,$2,$(module)))

endef
