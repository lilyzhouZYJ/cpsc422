# -*-Makefile-*-

OBJDIRS		+= $(USER_OBJDIR)/fstest

USER_FSTEST_SRC	+= $(USER_DIR)/fstest/fstest.c
USER_FSTEST_OBJ	:= $(patsubst %.c, $(OBJDIR)/%.o, $(USER_FSTEST_SRC))
USER_FSTEST_OBJ	:= $(patsubst %.S, $(OBJDIR)/%.o, $(USER_FSTEST_OBJ))
KERN_BINFILES	+= $(USER_OBJDIR)/fstest/fstest

USER_FLOCKTEST_SRC	+= $(USER_DIR)/fstest/flocktest.c
USER_FLOCKTEST_OBJ	:= $(patsubst %.c, $(OBJDIR)/%.o, $(USER_FLOCKTEST_SRC))
USER_FLOCKTEST_OBJ	:= $(patsubst %.S, $(OBJDIR)/%.o, $(USER_FLOCKTEST_OBJ))
KERN_BINFILES	+= $(USER_OBJDIR)/fstest/flocktest

USER_FLOCK1_SRC	+= $(USER_DIR)/fstest/flock1.c
USER_FLOCK1_OBJ	:= $(patsubst %.c, $(OBJDIR)/%.o, $(USER_FLOCK1_SRC))
USER_FLOCK1_OBJ	:= $(patsubst %.S, $(OBJDIR)/%.o, $(USER_FLOCK1_OBJ))
KERN_BINFILES	+= $(USER_OBJDIR)/fstest/flock1

USER_FLOCK2_SRC	+= $(USER_DIR)/fstest/flock2.c
USER_FLOCK2_OBJ	:= $(patsubst %.c, $(OBJDIR)/%.o, $(USER_FLOCK2_SRC))
USER_FLOCK2_OBJ	:= $(patsubst %.S, $(OBJDIR)/%.o, $(USER_FLOCK2_OBJ))
KERN_BINFILES	+= $(USER_OBJDIR)/fstest/flock2

USER_FLOCK3_SRC	+= $(USER_DIR)/fstest/flock3.c
USER_FLOCK3_OBJ	:= $(patsubst %.c, $(OBJDIR)/%.o, $(USER_FLOCK3_SRC))
USER_FLOCK3_OBJ	:= $(patsubst %.S, $(OBJDIR)/%.o, $(USER_FLOCK3_OBJ))
KERN_BINFILES	+= $(USER_OBJDIR)/fstest/flock3

fstest: $(USER_OBJDIR)/fstest/fstest     \
        $(USER_OBJDIR)/fstest/flocktest  \
        $(USER_OBJDIR)/fstest/flock1     \
        $(USER_OBJDIR)/fstest/flock2     \
        $(USER_OBJDIR)/fstest/flock3     \

$(USER_OBJDIR)/fstest/fstest: $(USER_LIB_OBJ) $(USER_FSTEST_OBJ)
	@echo + ld[USER/fstest] $@
	$(V)$(LD) -o $@ $(USER_LDFLAGS) $(USER_LIB_OBJ) $(USER_FSTEST_OBJ) $(GCC_LIBS)
	mv $@ $@.bak
	$(V)$(OBJCOPY) --remove-section .note.gnu.property $@.bak $@
	$(V)$(OBJDUMP) -S $@ > $@.asm
	$(V)$(NM) -n $@ > $@.sym

$(USER_OBJDIR)/fstest/flocktest: $(USER_LIB_OBJ) $(USER_FLOCKTEST_OBJ)
	@echo + ld[USER/flocktest] $@
	$(V)$(LD) -o $@ $(USER_LDFLAGS) $(USER_LIB_OBJ) $(USER_FLOCKTEST_OBJ) $(GCC_LIBS)
	mv $@ $@.bak
	$(V)$(OBJCOPY) --remove-section .note.gnu.property $@.bak $@
	$(V)$(OBJDUMP) -S $@ > $@.asm
	$(V)$(NM) -n $@ > $@.sym

$(USER_OBJDIR)/fstest/flock1: $(USER_LIB_OBJ) $(USER_FLOCK1_OBJ)
	@echo + ld[USER/flock1] $@
	$(V)$(LD) -o $@ $(USER_LDFLAGS) $(USER_LIB_OBJ) $(USER_FLOCK1_OBJ) $(GCC_LIBS)
	mv $@ $@.bak
	$(V)$(OBJCOPY) --remove-section .note.gnu.property $@.bak $@
	$(V)$(OBJDUMP) -S $@ > $@.asm
	$(V)$(NM) -n $@ > $@.sym

$(USER_OBJDIR)/fstest/flock2: $(USER_LIB_OBJ) $(USER_FLOCK2_OBJ)
	@echo + ld[USER/flock2] $@
	$(V)$(LD) -o $@ $(USER_LDFLAGS) $(USER_LIB_OBJ) $(USER_FLOCK2_OBJ) $(GCC_LIBS)
	mv $@ $@.bak
	$(V)$(OBJCOPY) --remove-section .note.gnu.property $@.bak $@
	$(V)$(OBJDUMP) -S $@ > $@.asm
	$(V)$(NM) -n $@ > $@.sym

$(USER_OBJDIR)/fstest/flock3: $(USER_LIB_OBJ) $(USER_FLOCK3_OBJ)
	@echo + ld[USER/flock3] $@
	$(V)$(LD) -o $@ $(USER_LDFLAGS) $(USER_LIB_OBJ) $(USER_FLOCK3_OBJ) $(GCC_LIBS)
	mv $@ $@.bak
	$(V)$(OBJCOPY) --remove-section .note.gnu.property $@.bak $@
	$(V)$(OBJDUMP) -S $@ > $@.asm
	$(V)$(NM) -n $@ > $@.sym

$(USER_OBJDIR)/fstest/%.o: $(USER_DIR)/fstest/%.c
	@echo + cc[USER/fstest] $<
	@mkdir -p $(@D)
	$(V)$(CC) $(USER_CFLAGS) -c -o $@ $<

$(USER_OBJDIR)/fstest/%.o: $(USER_DIR)/fstest/%.S
	@echo + as[USER/fstest] $<
	@mkdir -p $(@D)
	$(V)$(CC) $(USER_CFLAGS) -c -o $@ $<