# -*-Makefile-*-

OBJDIRS += $(KERN_OBJDIR)/trap/BBQueue

KERN_SRCFILES += $(KERN_DIR)/trap/CondVariable/BBQueue.c

$(KERN_OBJDIR)/trap/BBQueue/%.o: $(KERN_DIR)/trap/BBQueue/%.c
	@echo + $(COMP_NAME)[KERN/trap/BBQueue] $<
	@mkdir -p $(@D)
	$(V)$(CCOMP) $(CCOMP_KERN_CFLAGS) -c -o $@ $<

$(KERN_OBJDIR)/trap/BBQueue/%.o: $(KERN_DIR)/trap/BBQueue/%.S
	@echo + as[KERN/trap/BBQueue] $<
	@mkdir -p $(@D)
	$(V)$(CC) $(KERN_CFLAGS) -c -o $@ $<
