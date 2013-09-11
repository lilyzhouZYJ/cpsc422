# -*-Makefile-*-

OBJDIRS		+= $(KERN_OBJDIR)/kern/virt

KERN_SRCFILES	+= $(KERN_DIR)/kern/virt/vmcb.c
KERN_SRCFILES	+= $(KERN_DIR)/kern/virt/npt.c
KERN_SRCFILES	+= $(KERN_DIR)/kern/virt/svm.c
KERN_SRCFILES	+= $(KERN_DIR)/kern/virt/hvm.c

$(KERN_OBJDIR)/kern/virt/%.o: $(KERN_DIR)/kern/virt/%.c
	@echo + cc[KERN/kern] $<
	@mkdir -p $(@D)
	$(V)$(CC) $(KERN_CFLAGS) -c -o $@ $<

$(KERN_OBJDIR)/kern/virt/%.o: $(KERN_DIR)/kern/virt/%.S
	@echo + as[KERN/kern] $<
	@mkdir -p $(@D)
	$(V)$(CC) $(KERN_CFLAGS) -c -o $@ $<