product-support-types+=android

ANDROID_ARM_MODE?=arm

#$(call def-copy-dir-r,src-dir,target-dir,postfix-list)
android-asserts-def-copy-dir=android-asserts-def-sep copy-dir $1 $2 $3

.PHONY: android android-proj

# $(call product-def-rule-android,product-name,domain,src,target)
define product-def-rule-android-assert

auto-build-dirs+=$(dir $4)

$1.$2.android-proj: $4

$4: $3
	$$(call with_message,copy assert $(patsubst $(CPDE_OUTPUT_ROOT)/$2/%,%,$4))cp $$< $$@

endef

# $(call product-def-rule-android-asserts-copy-dir,product-name,domain,args)
define product-def-rule-android-asserts-copy-dir

$(foreach f,$(filter $(CPDE_OUTPUT_ROOT)/$2/$(word 1,$3)/%,$(r.$1.$2.installed-files)), \
	$(call product-def-rule-android-assert,$1,$2,$f,\
          $(patsubst $(CPDE_OUTPUT_ROOT)/$2/$(word 1,$3)/%,$(CPDE_OUTPUT_ROOT)/$($1.$2.android.asserts-dir)/$(if $(word 2, $3),$(word 2, $3)/%,)%,$f)))

endef

# $(call product-def-rule-android-gen-depends-projs,product-name,domain)
define product-def-rule-android-gen-depends-projs
$(foreach d,$(call product-gen-depend-list,$($2.env),$1),\
   $(if $(filter lib,$($d.type)),\
       $(if $(filter $d,$(android.$2.defined-projects)),,\
            $(eval android.$2.defined-projects+=$d) \
            $(call product-def-rule-android-rules,$d,$2) \
            $1.$2.android-proj: $d.$2.android-proj \
        ) \
    )) \

endef

# $(call product-def-rule-android,product-name,domain)
define product-def-rule-android-app
$(call assert-not-null,$1.android.manifest)

$(eval $1.$2.android.manifest:=$2/$1/AndroidManifest.xml)
$(eval $1.$2.android.application-mk:=$2/$1/jni/Application.mk)
$(eval $1.$2.android.project-properties:=$2/$1/project.properties)
$(eval $1.$2.android.asserts-dir:=$2/$1/asserts)

.PHONY: $1.$2.android $1.android $2.android

$2.android android $1.android: $1.$2.android

$1.$2.android: $1.$2.android-proj
	ndk-build --directory=$(CPDE_OUTPUT_ROOT)/$$($1.$2.android.output) $(if $(filter 1,$V),V=1) -k

$1.$2.android-proj: $(CPDE_OUTPUT_ROOT)/$$($1.$2.android.manifest)  $(CPDE_OUTPUT_ROOT)/$$($1.$2.android.application-mk) $(CPDE_OUTPUT_ROOT)/$$($1.$2.android.project-properties)

$(CPDE_OUTPUT_ROOT)/$$($1.$2.android.project-properties):
	$$(call with_message,generating $$($1.$2.android.android-mk))echo '# anto generate by makefile' >> $$@
	$$(CPE_SILENCE_TAG)echo 'target=android-8' > $$@

$(CPDE_OUTPUT_ROOT)/$$($1.$2.android.manifest): $(r.$1.base)/$($1.android.manifest)
	$$(call with_message,generating manifest)cp $$< $$@

$(CPDE_OUTPUT_ROOT)/$$($1.$2.android.application-mk):
	$$(call with_message,generating $$($1.$2.android.android-mk))echo '# anto generate by makefile' >> $$@
	$$(CPE_SILENCE_TAG)echo 'LOCAL_PATH := $$$$(call my-dir)' > $$@
	$$(CPE_SILENCE_TAG)echo '' >> $$@
	$$(CPE_SILENCE_TAG)echo 'APP_STL := gnustl_static' >> $$@
	$$(CPE_SILENCE_TAG)echo 'APP_ABI := armeabi' >> $$@
	$$(CPE_SILENCE_TAG)echo 'APP_OPTIM := debug' >> $$@

$(eval product-def-rule-android-asserts-tmp-name:=)
$(eval product-def-rule-android-asserts-tmp-args:=)

$(foreach w,$($1.android.asserts), \
    $(if $(filter android-asserts-def-sep,$w) \
        , $(if $(product-def-rule-android-asserts-tmp-name) \
              , $(call product-def-rule-android-asserts-$(product-def-rule-android-asserts-tmp-name),$1,$2,$(product-def-rule-android-asserts-tmp-args)) \
                $(eval product-def-rule-android-asserts-tmp-name:=) \
                $(eval product-def-rule-android-asserts-tmp-args:=)) \
        , $(if $(product-def-rule-android-asserts-tmp-name) \
              , $(eval product-def-rule-android-asserts-tmp-args+=$w) \
              , $(eval product-def-rule-android-asserts-tmp-name:=$w))))

$(if $(product-def-rule-android-asserts-tmp-name) \
    , $(call product-def-rule-android-asserts-$(product-def-rule-android-asserts-tmp-name),$1,$2,$(product-def-rule-android-asserts-tmp-args)))

endef

# $(call product-def-rule-android-rules,product-name,domain)
define product-def-rule-android-rules

$(if $(filter $1,$(android.$2.project-list)),$(warning $1 is already installed in android.$2),$(eval android.$2.project-list+=$1))

$(eval $1.$2.android.output:=$2/$1)
$(eval $1.$2.android.android-mk:=$2/$1/jni/Android.mk)
$(eval $1.$2.android.srcs:=$(subst $(CPDE_ROOT),../../../..,\
                           $(subst $(CPDE_OUTPUT_ROOT),../../..,\
                                   $(r.$1.c.sources) $(r.$1.$($2.env).c.sources) $(r.$1.$2.c.sources))))

auto-build-dirs+=$$(CPDE_OUTPUT_ROOT)/$$(dir $$($1.$2.android.android-mk))

.PHONY: $2.android-proj $1.android-proj $1.$2.android-proj

$2.android-proj android-proj: $1.$2.android-proj

$1.$2.android-proj: $(CPDE_OUTPUT_ROOT)/$$($1.$2.android.android-mk) $$(r.$1.$2.generated-sources)

$(CPDE_OUTPUT_ROOT)/$$($1.$2.android.android-mk): $$(r.$1.c.sources) $$(r.$1.$($2.env).c.sources) $$(r.$1.$2.c.sources))))
	$$(call with_message,generating $$($1.$2.android.android-mk))echo '# anto generate by makefile' >> $$@
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
$(eval $2-android.ignore-types:=android progn lib)

$(call product-def-for-domain,$1,$2-android)

$(call product-def-rule-android-rules,$1,$2-android)

$(call post-commands-add,product-def-rule-android-gen-depends-projs,$1,$2-android)

endef