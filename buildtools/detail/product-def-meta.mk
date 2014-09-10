# {{{ 注册到product-def.mk
product-support-types+=meta
# }}}
# {{{ 定义meta转换
# $(call def-meta-convert,product,excell,meta,output)

define def-meta-convert

$1.meta.$(basename $4).input-file:=$(word 1, $(subst ., ,$2)).xls
$1.meta.$(basename $4).input-sheet:=$(word 2, $(subst ., ,$2))
$1.meta.$(basename $4).output:=$4
$1.meta.$(basename $4).meta-lib:=$(word 1, $(subst ., ,$3)).xml
$1.meta.$(basename $4).meta-name:=$(word 2, $(subst ., ,$3))

$(eval $1.meta.list+=$(basename $4))

endef

# }}}
# {{{ 实现：meta导出操作

#$(call product-def-rule-meta,product,module)
define product-def-rule-meta-convert

.POHEY: $$($1.meta.output)/$$($1.meta.$2.output)
.DELETE_ON_ERROR: $$($1.meta.output)/$$($1.meta.$2.output)

$1-meta: $$($1.meta.output)/$$($1.meta.$2.output)

$$($1.meta.output)/$$($1.meta.$2.output): $$($1.meta.input)/$$($1.meta.$2.input-file) $$($1.meta.config)/$$($1.meta.$2.meta-lib)
	$(CPDE_PERL) $$(CPDE_ROOT)/buildtools/tools/gen-meta.pl \
        --input-file $$^ \
        --input-sheet $$($1.meta.$2.input-sheet) \
        --output $$@ \
        --meta-lib=$$($1.meta.config)/$$($1.meta.$2.meta-lib) \
        --meta-name=$$($1.meta.$2.meta-name) \
        --start-line=$$($1.meta.start-line)

endef

# }}}
# {{{ 实现：international导出操作

#$(call product-def-rule-meta-international,product)
define product-def-rule-meta-international

$(call assert-not-null,$1.meta.languages)
$(call assert-not-null,$1.meta.international-inputs)

.POHEY: $$($1.meta.output)/strings_$$(firstword $$($1.meta.languages)).stb

.DELETE_ON_ERROR: $$($1.meta.output)/strings_$$(firstword $$($1.meta.languages)).stb

$1-meta: $$($1.meta.output)/strings_$$(firstword $$($1.meta.languages)).stb

$$($1.meta.output)/strings_$$(firstword $$($1.meta.languages)).stb: \
	$$(foreach f,$$($1.meta.international-inputs),$$($1.meta.input)/$$(firstword $$(subst ., ,$$f)).xls)
	$(CPDE_PERL) $$(CPDE_ROOT)/buildtools/tools/gen-strings.pl \
        $$(addprefix --input $$($1.meta.input)/, $$($1.meta.international-inputs)) \
        $$(addprefix --language ,$$($1.meta.languages)) \
        --output $$($1.meta.output)/strings \
        --start-line=$$($1.meta.start-line)

endef

# }}}
# {{{ 实现：定义所有转换操作

#$(call product-def-rule-meta,product)
define product-def-rule-meta-all

.POHEY: $1 $1-meta

$1: $1-meta

$(call assert-not-null,$1.meta.start-line)
$(call assert-not-null,$1.meta.output)
$(call assert-not-null,$1.meta.input)
$(call assert-not-null,$1.meta.config)

auto-build-dirs+=$$($1.meta.output)

$(eval $(foreach f,$($1.meta.list),$(call product-def-rule-meta-convert,$1,$f)))
$(if $($1.meta.international-inputs)$($($1.meta.languages)),$(call product-def-rule-meta-international,$1))

endef

# }}}
# {{{ 实现：总入口
define product-def-rule-meta

$(if $($1-rule-meta-defined),,$(eval $1-rule-meta-defined:=1)$(call product-def-rule-meta-all,$1))

endef

# }}}
