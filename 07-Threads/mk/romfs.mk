ROMDIR = test-romfs
DAT += test-romfs.o

$(ROMDIR).o: $(ROMDIR).bin
	@mkdir -p $(dir $@)
	@echo "    OBJCOPY "$@
	@$(CROSS_COMPILE)objcopy -I binary -O elf32-littlearm -B arm \
		--prefix-sections '.romfs' $< $@

$(ROMDIR).bin: $(ROMDIR) mkromfs
	@mkdir -p $(dir $@)
	@echo "    MKROMFS "$@
	@./mkromfs -d $< $@

$(ROMDIR):
	@mkdir -p $@

mkromfs: mkromfs.c
	@mkdir -p $(dir $@)
	@echo "    CC      "$@
	@gcc -Wall -o $@ $^
