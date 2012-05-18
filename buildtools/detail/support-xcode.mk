# {{{ xcode相关命令行定义
IBTOOL=/usr/bin/ibtool
# }}}
# {{{ xcode相关的转换工具定义
tools.cvt.xib-nib.cmd=$(IBTOOL) --compile $$@ $$<
tools.cvt.xib-nib.dep=
# }}}