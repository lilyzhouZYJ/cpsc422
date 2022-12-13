# -*-Makefile-*-

OBJDIRS += $(KERN_OBJDIR)/vmm/MPTCOW

KERN_SRCFILES += $(KERN_DIR)/vmm/MPTCOW/MPTCOW.c
ifdef TEST
KERN_SRCFILES += $(KERN_DIR)/vmm/MPTCOW/test.c
endif

$(KERN_OBJDIR)/vmm/MPTCOW/%.o: $(KERN_DIR)/vmm/MPTCOW/%.c
	@echo + $(COMP_NAME)[KERN/vmm/MPTCOW] $<
	@mkdir -p $(@D)
	$(V)$(CCOMP) $(CCOMP_KERN_CFLAGS) -c -o $@ $<

$(KERN_OBJDIR)/vmm/MPTCOW/%.o: $(KERN_DIR)/vmm/MPTCOW/%.S
	@echo + as[KERN/vmm/MPTCOW] $<
	@mkdir -p $(@D)
	$(V)$(CC) $(KERN_CFLAGS) -c -o $@ $<
