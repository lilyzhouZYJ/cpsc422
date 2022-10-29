# -*-Makefile-*-

OBJDIRS	+= $(KERN_OBJDIR)/thread/CVThreadQueue

KERN_SRCFILES += $(KERN_DIR)/thread/CVThreadQueue/CVThreadQueue.c
ifdef TEST
KERN_SRCFILES += $(KERN_DIR)/thread/CVThreadQueue/test.c
endif

$(KERN_OBJDIR)/thread/CVThreadQueue/%.o: $(KERN_DIR)/thread/CVThreadQueue/%.c
	@echo + $(COMP_NAME)[KERN/thread/CVThreadQueue] $<
	@mkdir -p $(@D)
	$(V)$(CCOMP) $(CCOMP_KERN_CFLAGS) -c -o $@ $<

$(KERN_OBJDIR)/thread/CVThreadQueue/%.o: $(KERN_DIR)/thread/CVThreadQueue/%.S
	@echo + as[KERN/thread/CVThreadQueue] $<
	@mkdir -p $(@D)
	$(V)$(CC) $(KERN_CFLAGS) -c -o $@ $<
