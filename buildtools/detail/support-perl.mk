CPDE_PERL_LIB:=$(CPDE_ROOT)/buildtools/perl
CPDE_PERL=PERL5LIB=$(call path-list-join,$(CPDE_PERL_LIB)) perl -w