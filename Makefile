RM = rm -f
TAR = tar
MKDIR = mkdir -p
LN = cp -r
GZIP = GZIP=-9 gzip
ARCHIVE = dama2008

DISTFILES = src doc

all:
	$(MAKE) -C src

pack: $(ARCHIVE).tar.gz

$(ARCHIVE).tar.gz: $(DISTFILES)
	-$(RM) -rf $(ARCHIVE)
	$(MKDIR) $(ARCHIVE)
	$(LN) $(DISTFILES) $(ARCHIVE)/
	$(TAR) cf - $(ARCHIVE) | $(GZIP) > $(ARCHIVE).tar.gz
	-$(RM) -rf $(ARCHIVE)
