.PHONY: install

installdir=/usr/include/garbaz/
cmd_makedir=mkdir -p
cmd_copy=cp

install: netlib.h
ifeq ($(wildcard $(installdir).),)
	$(cmd_makedir) $(installdir)
endif
	$(cmd_copy) netlib.h $(installdir)
