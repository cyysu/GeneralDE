product-support-types+=cpe-pom
product-def-all-items+=cpe-pom.modules

cpe-pom-tool=$(CPDE_OUTPUT_ROOT)/$(tools.output)/bin/cpe_pom_tool

define product-def-rule-cpe-pom-c-module-metalib-xml
  $(call assert-not-null,$1.cpe-pom.$2.metalib-xml.output)

  $(eval r.$1.$3.cpe-pom.$2.metalib-xml.output:=$($1.cpe-pom.$2.metalib-xml.output))
  $(eval r.$1.$3.cpe-pom.$2.metalib-xml.output-dir:=$(CPDE_OUTPUT_ROOT)/$3/$(dir $($1.cpe-pom.$2.metalib-xml.output)))
  $(eval r.$1.$3.cpe-pom.$2.generated.metalib-xml:=$(r.$1.$3.cpe-pom.$2.metalib-xml.output-dir)$(notdir $($1.cpe-pom.$2.metalib-xml.output)))

  auto-build-dirs += $(r.$1.$3.cpe-pom.$2.metalib-xml.output-dir)

  $(eval r.$1.cleanup += $(r.$1.$3.cpe-pom.$2.generated.metalib-xml))

  $(r.$1.$3.cpe-pom.$2.generated.metalib-xml): $(r.$1.$3.cpe-pom.$2.pom-meta-source) $(r.$1.$3.cpe-pom.$2.dr-meta-source) $(cpe-pom-tool)
	$$(call with_message,cpe-pom generaing metalib-xml to $(subst $(CPDE_ROOT)/,,$(r.$1.$3.cpe-pom.$2.generated.metalib-xml)) ...) \
	LD_LIBRARY_PATH=$(CPDE_OUTPUT_ROOT)/$(tools.output)/lib:$$$$LD_LIBRARY_PATH \
	$(cpe-pom-tool) metalib-xml \
                    $(addprefix --pom-meta , $(r.$1.$3.cpe-pom.$2.pom-meta-source)) \
                    $(addprefix --dr-meta , $(r.$1.$3.cpe-pom.$2.dr-meta-source)) \
                    --output-metalib-xml $$@
endef

define product-def-rule-cpe-pom-c-module-store-metalib-xml
  $(call assert-not-null,$1.cpe-pom.$2.store-metalib-xml.output)
  $(call assert-not-null,$1.cpe-pom.$2.store-metalib-xml.main-entry)
  $(call assert-not-null,$1.cpe-pom.$2.store-metalib-xml.key)

  $(eval r.$1.$3.cpe-pom.$2.store-metalib-xml.output:=$($1.cpe-pom.$2.store-metalib-xml.output))
  $(eval r.$1.$3.cpe-pom.$2.store-metalib-xml.output-dir:=$(CPDE_OUTPUT_ROOT)/$3/$(dir $($1.cpe-pom.$2.store-metalib-xml.output)))
  $(eval r.$1.$3.cpe-pom.$2.generated.store-metalib-xml:=$(r.$1.$3.cpe-pom.$2.store-metalib-xml.output-dir)$(notdir $($1.cpe-pom.$2.store-metalib-xml.output)))

  auto-build-dirs += $(r.$1.$3.cpe-pom.$2.store-metalib-xml.output-dir)

  $(eval r.$1.cleanup += $(r.$1.$3.cpe-pom.$2.generated.store-metalib-xml))

  $(r.$1.$3.cpe-pom.$2.generated.store-metalib-xml): $(r.$1.$3.cpe-pom.$2.pom-meta-source) $(r.$1.$3.cpe-pom.$2.dr-meta-source) $(cpe-pom-tool)
	$$(call with_message,cpe-pom generaing store-metalib-xml to $(subst $(CPDE_ROOT)/,,$(r.$1.$3.cpe-pom.$2.generated.store-metalib-xml)) ...) \
	LD_LIBRARY_PATH=$(CPDE_OUTPUT_ROOT)/$(tools.output)/lib:$$$$LD_LIBRARY_PATH \
	$(cpe-pom-tool) store-metalib-xml \
                    $(addprefix --pom-meta , $(r.$1.$3.cpe-pom.$2.pom-meta-source)) \
                    $(addprefix --dr-meta , $(r.$1.$3.cpe-pom.$2.dr-meta-source)) \
                    --main-entry $($1.cpe-pom.$2.store-metalib-xml.main-entry) \
                    --key $($1.cpe-pom.$2.store-metalib-xml.key) \
                    --output-metalib-xml $$@
endef

define product-def-rule-cpe-pom-c-module-c
  $(call assert-not-null,$1.cpe-pom.$2.c.output)
  $(call assert-not-null,$1.cpe-pom.$2.c.arg-name)

  $(eval r.$1.$3.cpe-pom.$2.c.output:=$($1.cpe-pom.$2.c.output))
  $(eval r.$1.$3.cpe-pom.$2.c.arg-name:=$($1.cpe-pom.$2.c.arg-name))
  $(eval r.$1.$3.cpe-pom.$2.c.output-dir:=$(call c-source-dir-to-binary-dir,$(r.$1.base)/$(patsubst %/,%,$(dir $(r.$1.$3.cpe-pom.$2.c.output))),$3))
  $(eval r.$1.$3.cpe-pom.$2.generated.c:=$(r.$1.$3.cpe-pom.$2.c.output-dir)/$(notdir $($1.cpe-pom.$2.c.output)))

  auto-build-dirs += $(r.$1.$3.cpe-pom.$2.c.output-dir)

  $(eval r.$1.$3.c.sources += $(r.$1.$3.cpe-pom.$2.generated.c))
  $(eval r.$1.cleanup += $(r.$1.$3.cpe-pom.$2.generated.c))

  $(r.$1.$3.cpe-pom.$2.generated.c): $(r.$1.$3.cpe-pom.$2.pom-meta-source) $(r.$1.$3.cpe-pom.$2.dr-meta-source) $(cpe-pom-tool)
	$$(call with_message,cpe-pom generaing lib-c to $(subst $(CPDE_ROOT)/,,$(r.$1.$3.cpe-pom.$2.generated.c)) ...) \
	LD_LIBRARY_PATH=$(CPDE_OUTPUT_ROOT)/$(tools.output)/lib:$$$$LD_LIBRARY_PATH \
	$(cpe-pom-tool) mk-clib \
                    $(addprefix --pom-meta , $(r.$1.$3.cpe-pom.$2.pom-meta-source)) \
                    $(addprefix --dr-meta , $(r.$1.$3.cpe-pom.$2.dr-meta-source)) \
                    --page-size $(r.$1.$3.cpe-pom.$2.page-size) \
                    --output-lib-c $$@ --output-lib-c-arg $($1.cpe-pom.$2.c.arg-name)

endef

define product-def-rule-cpe-pom-c-module-hpp
  $(call assert-not-null,$1.cpe-pom.$2.hpp.output)
  $(call assert-not-null,$1.cpe-pom.$2.hpp.class-name)

  $(eval r.$1.$3.cpe-pom.$2.hpp.output:=$($1.cpe-pom.$2.hpp.output))
  $(eval r.$1.$3.cpe-pom.$2.hpp.class-name:=$($1.cpe-pom.$2.hpp.class-name))
  $(eval r.$1.$3.cpe-pom.$2.hpp.namespace:=$($1.cpe-pom.$2.hpp.namespace))
  $(eval r.$1.$3.cpe-pom.$2.generated.hpp:=$(r.$1.base)/$($1.cpe-pom.$2.hpp.output))

  $(call c-source-to-object,$(r.$1.c.sources),$3): $(r.$1.$3.cpe-pom.$2.generated.hpp)

  $(r.$1.$3.cpe-pom.$2.generated.hpp): $(r.$1.$3.cpe-pom.$2.pom-meta-source) $(r.$1.$3.cpe-pom.$2.dr-meta-source) $(cpe-pom-tool)
	$$(call with_message,cpe-pom generaing hpp to $(subst $(CPDE_ROOT)/,,$(r.$1.$3.cpe-pom.$2.generated.hpp)) ...) \
	LD_LIBRARY_PATH=$(CPDE_OUTPUT_ROOT)/$(tools.output)/lib:$$$$LD_LIBRARY_PATH \
	$(cpe-pom-tool) mk-hpp \
                    $(addprefix --pom-meta , $(r.$1.$3.cpe-pom.$2.pom-meta-source)) \
                    $(addprefix --dr-meta , $(r.$1.$3.cpe-pom.$2.dr-meta-source)) \
                    --output-hpp $$@ \
                    --class-name $($1.cpe-pom.$2.hpp.class-name) \
                    $(addprefix --namespace ,$($1.cpe-pom.$2.hpp.namespace))

endef

define product-def-rule-cpe-pom-c-module
$(call assert-not-null,$1.cpe-pom.$3.pom-meta-source)
$(call assert-not-null,$1.cpe-pom.$3.page-size)
$(call assert-not-null,$1.cpe-pom.$3.dr-meta-source)
$(call assert-not-null,$1.cpe-pom.$3.generate)

$(eval r.$1.$2.cpe-pom.$3.pom-meta-source:=$($1.cpe-pom.$3.pom-meta-source))
$(eval r.$1.$2.cpe-pom.$3.page-size:=$($1.cpe-pom.$3.page-size))
$(eval r.$1.$2.cpe-pom.$3.dr-meta-source:=$($1.cpe-pom.$3.dr-meta-source))
$(eval r.$1.$2.cpe-pom.$3.generate:=$($1.cpe-pom.$3.generate))

$(foreach p,$(r.$1.$2.cpe-pom.$3.generate), $(call product-def-rule-cpe-pom-c-module-$p,$1,$3,$2))

endef

define product-def-rule-cpe-pom

$(foreach module,$(r.$1.cpe-pom.modules),\
	$(call product-def-rule-cpe-pom-c-module,$1,$2,$(module)))

endef
