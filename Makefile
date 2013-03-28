MAKEFLAGS += -rR --no-print-directory


OBJS := ff_blk.o
OBJS += ff_crc.o
OBJS += ff_dir.o
OBJS += ff_error.o
OBJS += ff_fat.o
OBJS += ff_file.o
OBJS += ff_format.o
OBJS += ff_hash.o
OBJS += ff_ioman.o 
OBJS += ff_memory.o
OBJS += ff_safety.o
OBJS += ff_string.o
OBJS += ff_time.o
OBJS += ff_unicode.o

DEPFOLDER = .deps

# link
libfullfat.a: $(OBJS)
	@echo "  [AR]\tlibfullfat.a"
	@ar rvs libfullfat.a $(OBJS) 1> /dev/null 2> /dev/null

# pull in dependency info for *existing* .o files
-include $(OBJS:.o=.d)

# compile and generate dependency info;
# more complicated dependency computation, so all prereqs listed
# will also become command-less, prereq-less targets
#   sed:    strip the target (everything before colon)
#   sed:    remove any continuation backslashes
#   fmt -1: list words one per line
#   sed:    strip leading spaces
#   sed:    add trailing colons
%.o: %.c
	@echo "  [CC]\t$*.c"
	gcc -Wall -c -g $(CFLAGS) $*.c -o $*.o
	@gcc -MM $(CFLAGS) $*.c > $*.d
	@cp -f $*.d $*.d.tmp
	@sed -e 's/.*://' -e 's/\\$$//' < $*.d.tmp | fmt -1 | \
	  sed -e 's/^ *//' -e 's/$$/:/' >> $*.d
	@rm -f $*.d.tmp

# remove compilation products
clean:
	@rm -f *.a *.o *.d
