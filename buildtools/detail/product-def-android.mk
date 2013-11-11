product-support-types+=android
product-def-all-items+=android.java-dir

ANDROID_ARM_MODE?=arm

#$(call android-proj-copy-dir,src-dir,target-dir,postfix-list)
android-proj-copy-dir=copy-dir $1 $2 $3 android-proj-def-sep
android-proj-copy-file=copy-file $1 $2 android-proj-def-sep
android-proj-combine-etc=combine-etc $1 $2 $3 android-proj-def-sep

.PHONY: android android.proj

# $(call product-def-rule-android-proj-copy,product-name,domain,src,target)
define product-def-rule-android-proj-copy

auto-build-dirs+=$(dir $4)

$1.$2.android.proj: $4

$4: $3
	$$(call with_message,copy $(patsubst $(CPDE_OUTPUT_ROOT)/$(if $2,$2/)%,%,$4))cp $$< $$@

endef

# $(call product-def-rule-android-proj-copy-dir,product-name,domain,args)
define product-def-rule-android-proj-copy-dir

$(call install-def-rule-one-dir-r,$1,$(strip $(word 1,$3)),$($1.$2.android.output)/$(strip $(word 2,$3)),$(wordlist 3,$(words $3),$3),$2,$1.$2.android.proj)

endef

# $(call product-def-rule-android-proj-copy-file,product-name,domain,args)
define product-def-rule-android-proj-copy-file

$(call product-def-rule-android-proj-copy,$1,$2,$(CPDE_ROOT)/$(strip $(word 1,$3)),$(CPDE_OUTPUT_ROOT)/$($1.$2.android.output)/$(strip $(word 2,$3)))

endef

# $(call product-def-rule-android-proj-combine-etc,product-name,domain,args)
define product-def-rule-android-proj-combine-etc

$1.$2.android.proj: $(CPDE_OUTPUT_ROOT)/$($1.$2.android.output)/$(strip $(word 2,$3))

$(CPDE_OUTPUT_ROOT)/$($1.$2.android.output)/$(strip $(word 2,$3)): $$(CPDE_OUTPUT_ROOT)/tools/bin/cpe_cfg_tool $(shell find $(CPDE_ROOT)/$(word 1,$3) -name "*.y[a]ml")
	$$(call with_message,combine $(word 1, $3) to $(word 2, $3))$$(CPDE_OUTPUT_ROOT)/tools/bin/cpe_cfg_tool combine --input $$(CPDE_ROOT)/$(word 1,$3) --output $$@

endef

# $(call product-def-rule-android-gen-dep,dep,dep-to,domain)
define product-def-rule-android-gen-dep
$1.$3.android.proj: $2.$3.android.proj

endef

# $(call product-def-rule-android-gen-depends-projs,product-name,domain)
define product-def-rule-android-gen-depends-projs
$(foreach d,$(call product-gen-depend-list,$($2.env),$1),\
   $(if $(filter lib,$($d.type)),\
       $(if $(filter $d,$(android.$2.defined-projects)),,\
            $(eval android.$2.defined-projects+=$d) \
            $(call product-def-rule-android-rules,$d,$2) \
        ) \
        $(call product-def-rule-android-gen-dep,$1,$d,$2) \
    )) \

endef

# $(call product-def-rule-android-gen-java-rules,product-name,domain)
define product-def-rule-android-gen-java-rules

$(foreach d,$(r.$1.android.java-dir) $(call product-gen-depend-value-list,$1,$($2.env),android.java-dir),\
    $(foreach j,$(shell find $d -name "*.java"),\
        $(call product-def-rule-android-proj-copy,$1,$2,$j,$(patsubst $d/%,$(CPDE_OUTPUT_ROOT)/$($1.$2.android.output)/src/%,$j))))

endef

# $(call product-def-rule-android,product-name,domain)
define product-def-rule-android-app
$(eval $1.$2.android.stl?=gnustl_static)

.PHONY: $1.$2.android $1.android $2.android $1.$2.android.native

$2.android.apk android.apk $1.android.apk: $1.$2.android.apk

$1.$2.android.apk: $1.$2.android.native
	ant -f $$(CPDE_OUTPUT_ROOT)/$$($1.$2.android.output)/build.xml $(if $(filter 0,$(APKD)),release,debug)

$2.android.install android.install $1.android.install: $1.$2.android.install

$1.$2.android.install: $(if $(filter 1,$(only)),,$1.$2.android.apk)
	ant -f $$(CPDE_OUTPUT_ROOT)/$$($1.$2.android.output)/build.xml $(if $(filter 0,$(APKD)),installr,installd)

$1.$2.android.native: $(if $(filter 1,$(only)),,$1.$2.android.proj)
	ndk-build --directory=$$(CPDE_OUTPUT_ROOT)/$$($1.$2.android.output) $$(if $$(filter 1,$$V),V=1) -k

$1.$2.android.proj: $(CPDE_OUTPUT_ROOT)/$($1.$2.android.output)/local.properties \
                    $(CPDE_OUTPUT_ROOT)/$($1.$2.android.output)/jni/Application.mk \
                    $(CPDE_OUTPUT_ROOT)/$($1.$2.android.output)/project.properties

$(CPDE_OUTPUT_ROOT)/$$($1.$2.android.output)/project.properties:
	$$(call with_message,$1.$2 <== generating $$(notdir $$@))echo '# anto generate by makefile' >> $$@
	$$(CPE_SILENCE_TAG)echo 'target=android-8' > $$@

$$(CPDE_OUTPUT_ROOT)/$$($1.$2.android.output)/local.properties:
	$$(call with_message,$1.$2 <== generating $$(notdir $$@))echo "sdk.dir=$(ANDROID_SDK_ROOT)" >> $$@

$(CPDE_OUTPUT_ROOT)/$$($1.$2.android.output)/jni/Application.mk:
	$$(call with_message,$1.$2 <== generating $$(notdir $$@))echo '# anto generate by makefile' >> $$@
	$$(CPE_SILENCE_TAG)echo 'LOCAL_PATH := $$$$(call my-dir)' > $$@
	$$(CPE_SILENCE_TAG)echo '' >> $$@
	$$(CPE_SILENCE_TAG)echo 'APP_STL := $$($1.$2.android.stl)' >> $$@
	$$(CPE_SILENCE_TAG)echo 'APP_ABI := armeabi' >> $$@
	$$(CPE_SILENCE_TAG)echo 'APP_OPTIM := debug' >> $$@

$(eval product-def-rule-android-proj-tmp-name:=)
$(eval product-def-rule-android-proj-tmp-args:=)

$(foreach w,$($1.android.proj-src), \
    $(if $(filter android-proj-def-sep,$w) \
        , $(if $(product-def-rule-android-proj-tmp-name) \
              , $(call product-def-rule-android-proj-$(product-def-rule-android-proj-tmp-name),$1,$2,$(product-def-rule-android-proj-tmp-args)) \
                $(eval product-def-rule-android-proj-tmp-name:=) \
                $(eval product-def-rule-android-proj-tmp-args:=)) \
        , $(if $(product-def-rule-android-proj-tmp-name) \
              , $(eval product-def-rule-android-proj-tmp-args+=$w) \
              , $(eval product-def-rule-android-proj-tmp-name:=$w))))

endef

# $(call product-def-rule-android-rules,product-name,domain)
define product-def-rule-android-rules

$(if $(filter $1,$(android.$2.project-list)),$(warning $1 is already installed in android.$2),$(eval android.$2.project-list+=$1))

$(eval $1.$2.android.output:=$2/$1)
$(eval $1.$2.android.srcs:=$(subst $(CPDE_ROOT),../../../..,\
                           $(subst $(CPDE_OUTPUT_ROOT),../../..,\
                                   $(r.$1.c.sources) $(r.$1.$($2.env).c.sources) $(r.$1.$2.c.sources))))

auto-build-dirs+=$$(CPDE_OUTPUT_ROOT)/$$($1.$2.android.output) $$(CPDE_OUTPUT_ROOT)/$$($1.$2.android.output)/jni

.PHONY: $2.android.proj $1.android.proj $1.$2.android.proj

$2.android.proj android.proj $1.android.proj: $1.$2.android.proj

$1.$2.android.proj: $(CPDE_OUTPUT_ROOT)/$($1.$2.android.output)/jni/Android.mk $$(r.$1.$2.generated-sources)

$(CPDE_OUTPUT_ROOT)/$($1.$2.android.output)/jni/Android.mk: $$(r.$1.c.sources) $$(r.$1.$($2.env).c.sources) $$(r.$1.$2.c.sources)
	$$(call with_message,$1.$2 <== generating $$(notdir $$@))echo '# anto generate by makefile' >> $$@
	$$(CPE_SILENCE_TAG)echo 'LOCAL_PATH := $$$$(call my-dir)' > $$@
	$$(CPE_SILENCE_TAG)echo '' >> $$@
	$$(CPE_SILENCE_TAG)echo 'include $$$$(CLEAR_VARS)' >> $$@
	$$(CPE_SILENCE_TAG)echo 'LOCAL_MODULE := $1' >> $$@
	$$(CPE_SILENCE_TAG)echo 'LOCAL_ARM_MODE := $(ANDROID_ARM_MODE)' >> $$@
	$$(CPE_SILENCE_TAG)echo 'LOCAL_CPP_FEATURES := rtti exceptions' >> $$@
	$$(CPE_SILENCE_TAG)echo 'LOCAL_CFLAGS += ' $$(filter -D%,$$(call c-generate-depend-cpp-flags,$1,$2)) >> $$@
	$$(CPE_SILENCE_TAG)echo 'LOCAL_C_INCLUDES += ' $$(patsubst -I%,%,$$(filter -I%,$$(call c-generate-depend-cpp-flags,$1,$2))) >> $$@
	$$(CPE_SILENCE_TAG)echo '' >> $$@
	$(if $(filter progn,$($1.type)),\
        $$(CPE_SILENCE_TAG)echo 'LOCAL_LDLIBS := -L$$$$(SYSROOT)/usr/lib $$($1.android.c.flags.ld)' \
                           >> $$@)
	$$(CPE_SILENCE_TAG)echo '' >> $$@
	$$(CPE_SILENCE_TAG)echo 'LOCAL_SRC_FILES += $($1.$2.android.srcs)' >> $$@
	$$(CPE_SILENCE_TAG)echo '' >> $$@
	$(if $(filter progn,$($1.type)),\
	    $$(CPE_SILENCE_TAG)echo 'LOCAL_WHOLE_STATIC_LIBRARIES := ' \
                              $$(foreach d,$$(call product-gen-depend-list,$($2.env),$1),$$(if $$(filter lib,$$($$d.type)),$$d)) \
                            >> $$@)
	$$(CPE_SILENCE_TAG)echo '' >> $$@
	$$(CPE_SILENCE_TAG)echo $(if $(filter progn,$($1.type)),'include $$$$(BUILD_SHARED_LIBRARY)','include $$$$(BUILD_STATIC_LIBRARY)') >> $$@
	$(if $(filter progn,$($1.type)),\
        $$(CPE_SILENCE_TAG)echo 'include ' \
                              $$(foreach d,$$(call product-gen-depend-list,$($2.env),$1),$$(if $$(filter lib,$$($$d.type)),$(CPDE_OUTPUT_ROOT)/$2/$$d/jni/Android.mk)) \
                            >> $$@)

$(if $(filter progn,$($1.type)),$(call product-def-rule-android-app,$1,$2))

endef

# $(call product-def-rule-android,product-name,domain)
define product-def-rule-android

$(eval $2-android.output?=$($2.output)-android)
$(eval $2-android.ut?=0)
$(eval $2-android.env:=android)
$(eval $2-android.ignore-types:=android progn lib install)

$(call product-def-for-domain,$1,$2-android)

$(call product-def-rule-android-rules,$1,$2-android)

$(call post-commands-add,product-def-rule-android-gen-depends-projs,$1,$2-android)

$(call post-commands-add,product-def-rule-android-gen-java-rules,$1,$2-android)

endef
