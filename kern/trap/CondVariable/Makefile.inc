# -*-Makefile-*-

OBJDIRS += $(KERN_OBJDIR)/trap/CondVariable

KERN_SRCFILES += $(KERN_DIR)/trap/CondVariable/CondVariable.c

$(KERN_OBJDIR)/trap/CondVariable/%.o: $(KERN_DIR)/trap/CondVariable/%.c
	@echo + $(COMP_NAME)[KERN/trap/CondVariable] $<
	@mkdir -p $(@D)
	$(V)$(CCOMP) $(CCOMP_KERN_CFLAGS) -c -o $@ $<

$(KERN_OBJDIR)/trap/CondVariable/%.o: $(KERN_DIR)/trap/CondVariable/%.S
	@echo + as[KERN/trap/CondVariable] $<
	@mkdir -p $(@D)
	$(V)$(CC) $(KERN_CFLAGS) -c -o $@ $<
