# {{{ xcode相关命令行定义
IBTOOL=/usr/bin/ibtool

#$(call iphone-simulator-find-run-path,app,simulator-version)
iphone-simulator-find-run-path=$(shell find "$(HOME)/Library/Application Support/iPhone Simulator/$(strip $2)/Applications" -maxdepth 2 -name '$(strip $1)')

# }}}
# {{{ xcode相关的转换工具定义
tools.cvt.xib-nib.cmd=$(IBTOOL) --compile $$@ $$<
tools.cvt.xib-nib.dep=
# }}}
# {{{ clang编译器命令行参数设定
compiler.clang.flags.warning=-Qunused-arguments -Wall -Wno-parentheses-equality
compiler.clang.flags.gen-dep=
# }}}
