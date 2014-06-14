# {{{ 注册到product-def.mk

product-support-types+=ui-model

UI_MODEL_TOOL=$(CPDE_ROOT)/build/tools/bin/$(call $(tools.env).make-executable-name,ui_model_tool)

# }}}
# {{{ 定义Module导入
#$(call install-ui-model-cocos-module,product,module,to-path,pics)
define install-ui-model-cocos-module

$(strip $1).ui-model.cocos-module.modules += $2
$(strip $1).ui-model.cocos-module.$(strip $2).input=$4
$(strip $1).ui-model.cocos-module.$(strip $2).output=$3

endef
# }}}
# {{{ 定义cocos序列帧特效

#$(call install-ui-model-cocos-effects-one,product,module,src-path,to-path,frame-duration,frame-position,frame-order)
define install-ui-model-cocos-effect-one
$1.ui-model.cocos-effect.modules+=$2
$1.ui-model.cocos-effect.$2.input=$3/$2
$1.ui-model.cocos-effect.$2.output=$(if $4,$4/$2,$2)
$1.ui-model.cocos-effect.$2.frame-duration=$5
$1.ui-model.cocos-effect.$2.frame-position=$6
$1.ui-model.cocos-effect.$2.frame-order=$7

endef

#$(call install-ui-model-cocos-effects,product,src-path,to-path,frame-duration,frame-position,frame-order)
define install-ui-model-cocos-effects
$(foreach m,$(basename $(notdir $(wildcard $($1.ui-model.root)/$2/*.plist))),$(call install-ui-model-cocos-effect-one,$1,$m,$2,$3,$4,$5,$6))

endef

# }}}
# {{{ 定义cocos2特效
#$(call install-ui-model-cocos-particles,product,module,src-path,to-path)
define install-ui-model-cocos-particle-one
$1.ui-model.cocos-particle.modules+=$2
$1.ui-model.cocos-particle.$2.input=$3/$2
$1.ui-model.cocos-particle.$2.output=$(if $4,$4/$2,$2)

endef

define install-ui-model-cocos-particles
$(foreach m,$(basename $(notdir $(wildcard $($1.ui-model.root)/$2/*.plist))),$(call install-ui-model-cocos-particle-one,$1,$m,$2,$3,$4))

endef
# }}}
# {{{ 定义file序列帧特效

#$(call install-ui-model-file-effects-one,product,module,to-path,input-path,pic-fils,frame-duration,frame-position,frame-order)
define install-ui-model-file-effect-one
$1.ui-model.file-effect.modules+=$2
$1.ui-model.file-effect.$2.pic-files=$5
$1.ui-model.file-effect.$2.input=$4
$1.ui-model.file-effect.$2.output=$3
$1.ui-model.file-effect.$2.frame-duration=$6
$1.ui-model.file-effect.$2.frame-position=$7
$1.ui-model.file-effect.$2.frame-order=$8

endef

#$(call install-ui-model-file-effects,product,to-path,src-path,sep,frame-duration,frame-position,frame-order)
define install-ui-model-file-effects

$(eval install-ui-model-file-effects-file-list:=$(basename $(notdir $(wildcard $3/*.png))))

$(foreach m,\
    $(sort $(foreach f,$(install-ui-model-file-effects-file-list),$(firstword $(subst $4, ,$f)))), \
    $(call install-ui-model-file-effect-one,$1,$m,$(if $2,$2/$m,$m),$3,$(filter $m$4%,$(install-ui-model-file-effects-file-list)),$5,$6,$7)) \


endef

#$(call install-ui-model-dir-effects,product,to-path,src-path,sep,frame-duration,frame-position,frame-order)
define install-ui-model-dir-effects

$(foreach m,\
	$(notdir $(wildcard $3/*)), \
        $(call install-ui-model-file-effect-one,$1,$(notdir $2)-$m,$(if $2,$2/$m,$m),$3/$m,$(basename $(notdir $(wildcard $3/$m/*.png))),$5,$6,$7)) \


endef

# }}}
# {{{ 实现：模型转换脚本

#$(call def-ui-model-tool-op,product,op,op-file)
define def-ui-model-tool-op

.PHONY: $1-model-op-$2
$1-model-op-$2: $$(UI_MODEL_TOOL)
	$(call with_message,do model op $2)$$(UI_MODEL_TOOL) manip --model $$($1.ui-model.root) --format np --op $3

endef

# }}}
# {{{ 实现：file序列帧导入

#$(call def-ui-model-file-effect,project,model)
define def-ui-model-file-effect-import

$$(call assert-not-null,TEXTUREPACKER)

$1-model-import: $$($1.ui-model.root)/Action/$$($1.ui-model.file-effect.$2.output)/$2.npAction

auto-build-dirs+=$$(dir $$($1.ui-model.root)/Texture/$$($1.ui-model.file-effect.$2.output)) \
                 $$(dir $$($1.ui-model.root)/Module/$$($1.ui-model.file-effect.$2.output)) \
                 $$(dir $$($1.ui-model.root)/Action/$$($1.ui-model.file-effect.$2.output))

.PHONY: $$($1.ui-model.file-effect.$2.input)/$2.tps
$$($1.ui-model.file-effect.$2.input)/$2.tps: $$(patsubst %,$$($1.ui-model.file-effect.$2.input)/%.png,$$($1.ui-model.file-effect.$2.pic-files))
	$(call with_message,generate tp project $2)$$(CPDE_PERL) $$(CPDE_ROOT)/buildtools/tools/gen-tp-project.pl \
        --output-proj $$@ \
        --output-pic $$(call build-relative-path,$$($1.ui-model.root)/Texture/$$($1.ui-model.file-effect.$2.output).png,$$($1.ui-model.file-effect.$2.input)/$2.tps) \
        --output-plist $2.plist \
        $$(addprefix --input , $$(sort $$(notdir $$^)))

$$($1.ui-model.file-effect.$2.input)/$2.plist: $$($1.ui-model.file-effect.$2.input)/$2.tps
	$(call with_message,texture pack $2)'$$(TEXTUREPACKER)' --force-publish '$$(call cygwin-path-to-win,$$^)'

$1.ui-model.file-effect.$2.frame-duration ?= 1
$1.ui-model.file-effect.$2.frame-position ?= center

$$($1.ui-model.root)/Action/$$($1.ui-model.file-effect.$2.output)/$2.npAction: $$($1.ui-model.file-effect.$2.input)/$2.plist $$(UI_MODEL_TOOL)
	$(call with_message,import file action $2)$$(UI_MODEL_TOOL) cocos-effect-import \
        --model $$($1.ui-model.root) --format np \
        --to-effect Action/$$($1.ui-model.file-effect.$2.output) \
        --to-module Module/$$($1.ui-model.file-effect.$2.output) \
        --frame-duration $$($1.ui-model.file-effect.$2.frame-duration) \
        --frame-position $$($1.ui-model.file-effect.$2.frame-position) \
        --frame-order $$($1.ui-model.file-effect.$2.frame-order) \
        --plist $$($1.ui-model.file-effect.$2.input)/$2.plist \
        --pic Texture/$$($1.ui-model.file-effect.$2.output).png

endef

# }}}
# {{{ 实现：cocos图片模块

#$(call def-ui-model-cocos-module-import,product,model)
define def-ui-model-cocos-module-import

$$(call assert-not-null,TEXTUREPACKER)
$$(call assert-not-null,$1.ui-model.cocos-module.$2.input)

$1-model-import: $$($1.ui-model.root)/Module/$$($1.ui-model.cocos-module.$2.output).npModule

auto-build-dirs+=$$(dir $$($1.ui-model.root)/Texture/$$($1.ui-model.cocos-module.$2.output).png) \
                 $$(dir $$($1.ui-model.root)/Module/$$($1.ui-model.cocos-module.$2.output).npModule)

.PHONY: $$(firstword $$(sort $$(dir $$($1.ui-model.cocos-module.$2.input))))$(strip $2).tps
$$(firstword $$(sort $$(dir $$($1.ui-model.cocos-module.$2.input))))$(strip $2).tps: $$($1.ui-model.cocos-module.$(strip $2).input)
	$(call with_message,generate tp project $2)$$(CPDE_PERL) $$(CPDE_ROOT)/buildtools/tools/gen-tp-project.pl \
        --output-proj $$@ \
        --output-pic $$(call build-relative-path,$$($1.ui-model.root)/Texture/$$($1.ui-model.cocos-module.$2.output).png,$$@) \
        --output-plist $$(call build-relative-path,$$($1.ui-model.root)/Texture/$$($1.ui-model.cocos-module.$2.output).plist,$$@) \
        $$(addprefix --input , $$(foreach i,$$(sort $$^),$$(call build-relative-path,$$i,$$(dir $$@))))

$$($1.ui-model.root)/Texture/$$($1.ui-model.cocos-module.$2.output).plist: $$(firstword $$(sort $$(dir $$($1.ui-model.cocos-module.$2.input))))$(strip $2).tps
	$(call with_message,texture pack $2)'$$(TEXTUREPACKER)' --force-publish '$$(call cygwin-path-to-win,$$^)'

$$($1.ui-model.root)/Module/$$($1.ui-model.cocos-module.$2.output).npModule: $$($1.ui-model.root)/Texture/$$($1.ui-model.cocos-module.$2.output).plist $$(UI_MODEL_TOOL)
	$(call with_message,import cocos module $2)$$(UI_MODEL_TOOL) cocos-module-import \
        --model $$($1.ui-model.root) --format np \
        --to-module Module/$$($1.ui-model.cocos-module.$2.output) \
        --plist $$($1.ui-model.root)/Texture/$$($1.ui-model.cocos-module.$2.output).plist \
        --pic Texture/$$($1.ui-model.cocos-module.$2.output).png

endef

# }}}
# {{{ 实现：cocos序列帧动画
#$(call def-ui-model-cocos-effect,model)
define def-ui-model-cocos-effect-import

$1-model-import: $$($1.ui-model.root)/Texture/$$($1.ui-model.cocos-effect.$2.output).png \
                 $$($1.ui-model.root)/Module/$$($1.ui-model.cocos-effect.$2.output).npModule \
                 $$($1.ui-model.root)/Action/$$($1.ui-model.cocos-effect.$2.output).npAction

auto-build-dirs+=$$(dir $$($1.ui-model.root)/Texture/$$($1.ui-model.cocos-effect.$2.output)) \
                 $$(dir $$($1.ui-model.root)/Module/$$($1.ui-model.cocos-effect.$2.output)) \
                 $$(dir $$($1.ui-model.root)/Action/$$($1.ui-model.cocos-effect.$2.output))

$$($1.ui-model.root)/Texture/$$($1.ui-model.cocos-effect.$2.output).png: $$($1.ui-model.root)/$$($1.ui-model.cocos-effect.$2.input).png
	cp $$< $$@

$1.ui-model.cocos-effect.$2.frame-duration ?= 1
$1.ui-model.cocos-effect.$2.frame-position ?= center
$1.ui-model.cocos-effect.$2.frame-order ?= native

.PHONY: $$($1.ui-model.root)/Action/$$($1.ui-model.cocos-effect.$2.output).npAction
$$($1.ui-model.root)/Module/$$($1.ui-model.cocos-effect.$2.output).npModule $$($1.ui-model.root)/Action/$$($1.ui-model.cocos-effect.$2.output).npAction: $$(wildcard $$($1.ui-model.root)/$$($1.ui-model.cocos-effect.$2.input).plist) $$(UI_MODEL_TOOL)
	$(call with_message,import cocos action $2)$$(UI_MODEL_TOOL) cocos-effect-import \
        --model $$($1.ui-model.root) --format np \
        --to-effect Action/$$($1.ui-model.cocos-effect.$2.output) \
        --to-module Module/$$($1.ui-model.cocos-effect.$2.output) \
        --frame-duration $$($1.ui-model.cocos-effect.$2.frame-duration) \
        --frame-position $$($1.ui-model.cocos-effect.$2.frame-position) \
        --frame-order $$($1.ui-model.cocos-effect.$2.frame-order) \
        --plist $$($1.ui-model.root)/$$($1.ui-model.cocos-effect.$2.input).plist \
        --pic Texture/$$($1.ui-model.cocos-effect.$2.output).png

endef
# }}}
# {{{ 实现：cocos粒子动画
#$(call def-ui-model-cocos-particle,model)
define def-ui-model-cocos-particle-import

$1-model-import: $$($1.ui-model.root)/Texture/$$($1.ui-model.cocos-particle.$2.output).png \
                 $$($1.ui-model.root)/Particle/$$($1.ui-model.cocos-particle.$2.output).particle

auto-build-dirs+=$$(dir $$($1.ui-model.root)/Texture/$$($1.ui-model.cocos-particle.$2.output)) \
                 $$(dir $$($1.ui-model.root)/Particle/$$($1.ui-model.cocos-particle.$2.output))

$$($1.ui-model.root)/Texture/$$($1.ui-model.cocos-particle.$2.output).png: $$($1.ui-model.root)/$$($1.ui-model.cocos-particle.$2.input).png
	cp $$< $$@

.PHONY: $$($1.ui-model.root)/Particle/$$($1.ui-model.cocos-particle.$2.output).particle
$$($1.ui-model.root)/Particle/$$($1.ui-model.cocos-particle.$2.output).particle: $$(wildcard $$($1.ui-model.root)/$$($1.ui-model.cocos-particle.$2.input).plist) $$(UI_MODEL_TOOL)
	$(call with_message,import cocos particle $2)$$(UI_MODEL_TOOL) cocos-particle-import \
        --model $$($1.ui-model.root) --format np \
        --to-particle Particle/$$($1.ui-model.cocos-particle.$2.output) \
        --plist $$($1.ui-model.root)/$$($1.ui-model.cocos-particle.$2.input).plist \
        --pic Texture/$$($1.ui-model.cocos-particle.$2.output).png

endef
# }}}
# {{{ 实现：总入口
#main interface
define product-def-rule-ui-model-i

.PHONY: $1-model-ops
$1-model-ops:
	@$(foreach op,$(patsubst %.yml,%,$(notdir $($1.ui-model.ops))),echo $1-model-op-$(op); )

.PHONY: $1-model-convert
$1-model-convert: $$(UI_MODEL_TOOL)
	$(call with_message,convert $1 ui model)$$(UI_MODEL_TOOL) convert --model $$($1.ui-model.root) --format np --to $$(CPDE_OUTPUT_ROOT)/model-output --to-format np

$(foreach op,$($1.ui-model.ops),$(call def-ui-model-tool-op,$1,$(patsubst %.yml,%,$(notdir $(op))),$(op)))


.PHONY: $1-model-import
$(foreach module,$($1.ui-model.file-effect.modules),$(call def-ui-model-file-effect-import,$1,$(module)))
$(foreach module,$($1.ui-model.cocos-module.modules),$(call def-ui-model-cocos-module-import,$1,$(module)))
$(foreach module,$($1.ui-model.cocos-effect.modules),$(call def-ui-model-cocos-effect-import,$1,$(module)))
$(foreach module,$($1.ui-model.cocos-particle.modules),$(call def-ui-model-cocos-particle-import,$1,$(module)))

endef

define product-def-rule-ui-model

$(if $($1-rule-ui-model-defined),,$(eval $1-rule-ui-model-defined:=1)$(call product-def-rule-ui-model-i,$1))

endef
# }}}
