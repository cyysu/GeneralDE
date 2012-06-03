# {{{ global settings

dev-env-list+=ios

ios.CFLAGS+=-Wall
ios.CXXFLAGS+=-Wall
ios.MFLAGS+=-Wall -pipe -x objective-c
ios.MMFLAGS+=-Wall -pipe -x objective-c++

ifneq ($(DEBUG),0)
ios.CFLAGS+=-g
ios.CXXFLAGS+=-g
endif

ios.LDFLAGS.share:=--shared

ios.default-lib-type:=static
ios.make-static-lib-name=lib$1.a
ios.make-dynamic-lib-name=lib$1.so
ios.make-executable-name=$1
ios.export-symbols=$(addprefix -u ,$(foreach m,$1,_$m))

ios.lib.iconv?=iconv
ios.lib.math?=m
ios.lib.dl?=dl

# }}}
# {{{ validate

ios.check=$(call assert-not-null,IOS_PLATFORM_NAME) \
             $(call assert-not-null,IOS_PLATFORM_VERSION) \
             $(call assert-not-null,$(IOS_PLATFORM_NAME).compiler) \
             $(call assert-not-null,$(IOS_PLATFORM_NAME).CFLAGS) \
             $(call assert-not-null,$(IOS_PLATFORM_NAME).CXXFLAGS) \
             $(call assert-not-null,$(IOS_PLATFORM_NAME).MFLAGS) \
             $(call assert-not-null,$(IOS_PLATFORM_NAME).MMFLAGS) \
             $(call assert-not-null,$(IOS_PLATFORM_NAME).LDFLAGS) \
             $(call assert-not-null,$(IOS_PLATFORM_NAME).TARGET_ARCH) \
             $(call assert-file-exists,$(IOS_SDK_PREFIX))

# }}}
# {{{ sdk iPhoneSimulator

ios-os-version-min?=40200

iPhoneSimulator.compiler ?= $(IOS_XCODE_ROOT)/Toolchains/XcodeDefault.xctoolchain/usr/bin/$(if $(filter gcc,$1),clang,$(if $(filter g++,$1),clang++,$1))

iPhoneSimulator.TARGET_ARCH ?= -arch i386

iPhoneSimulator.CPPFLAGS ?= \
                    -D__IPHONE_OS_VERSION_MIN_REQUIRED=$(ios-os-version-min) \
                    -DTARGET_IPHONE_SIMULATOR

iPhoneSimulator.CFLAGS ?= \
                   -std=c99 \
                   -fexceptions \
                   -mmacosx-version-min=10.6 \
                   -gdwarf-2 \

iPhoneSimulator.CXXFLAGS ?= \
                   -fexceptions \
                   -mmacosx-version-min=10.6 \
                   -gdwarf-2 \

iPhoneSimulator.MFLAGS ?= \
                   $(iPhoneSimulator.CFLAGS) \
                   -fmessage-length=0 \
                   -fpascal-strings \
                   -fasm-blocks \
                   -fobjc-abi-version=2 \
                   -fobjc-legacy-dispatch \

iPhoneSimulator.MMFLAGS ?= \
                   -fmessage-length=0 \
                   -fpascal-strings \
                   -fasm-blocks \
                   -fobjc-abi-version=2 \
                   -fobjc-legacy-dispatch \

iPhoneSimulator.LDFLAGS ?=  -arch i386\
                            -mmacosx-version-min=10.6 \
                            -Xlinker \
                            -objc_abi_version \
                            -Xlinker 2 

iPhoneSimulator.install-dir?=$(HOME)/Library/Application Support/iPhone Simulator/$(IOS_PLATFORM_VERSION)/Applications
iPhoneSimulator.run := 	echo ' \
application "iPhone Simulator" quit\n \
application "iPhone Simulator" activate\n \
' | osascript

# }}}
# {{{ toolset def

IOS_XCODE_ROOT:=$(if $(filter mac,$(OS_NAME)),$(shell xcode-select -print-path))
IOS_PLATFORM_PREFIX:=$(IOS_XCODE_ROOT)/Platforms/$(IOS_PLATFORM_NAME).platform
IOS_PLATFORM_BIN_PATH:=$(IOS_PLATFORM_PREFIX)/Developer/usr/bin
IOS_SDK_PREFIX:=$(IOS_PLATFORM_PREFIX)/Developer/SDKs/$(IOS_PLATFORM_NAME)$(IOS_PLATFORM_VERSION).sdk

ios.GCC = $(call $(IOS_PLATFORM_NAME).compiler,gcc)
ios.CXX = $(call $(IOS_PLATFORM_NAME).compiler,g++)
ios.CC = $(ios.GCC)
ios.LD = $(ios.CC)
ios.AR = $(IOS_PLATFORM_BIN_PATH)/ar
ios.STRIP = $(IOS_PLATFORM_BIN_PATH)/strip
ios.OBJCOPY = $(IOS_PLATFORM_BIN_PATH)/objcopy
ios.IBTOOL = ibtool

ios.IBFLAGS = \
          --errors \
          --warnings \
          --notices \
          --output-format human-readable-text \
          --sdk $(IOS_SDK_PREFIX)

ios.CPPFLAGS+=\
           -isysroot $(IOS_SDK_PREFIX) \
           -F$(IOS_SDK_PREFIX)/System/Library/Frameworks \
           $($(IOS_PLATFORM_NAME).CPPFLAGS)

ios.CFLAGS += $($(IOS_PLATFORM_NAME).CFLAGS)
ios.CXXFLAGS += $($(IOS_PLATFORM_NAME).CXXFLAGS)
ios.MFLAGS += $($(IOS_PLATFORM_NAME).MFLAGS)
ios.MMFLAGS += $($(IOS_PLATFORM_NAME).MMFLAGS)

ios.TARGET_ARCH += $($(IOS_PLATFORM_NAME).TARGET_ARCH)

ios.LDFLAGS += \
           -isysroot $(IOS_SDK_PREFIX) \
           $($(IOS_PLATFORM_NAME)LDFLAGS) \
           $(addprefix -L,$(sort $(dir $(libraries)))) \

ios.PLUTILFLAGS += -convert binary1

ios.linker.c:=$(ios.GCC)
ios.linker.cpp:=$(ios.CXX)
ios.linker.obj-c:=$(ios.GCC)
ios.linker.obj-cpp:=$(ios.CXX)

# }}}
