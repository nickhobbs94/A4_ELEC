LINK_TARGET = a4.out

OBJS =  \
 test.o \
 conversions.o \
 altstring.o


REBUILDABLES = $(LINK_TARGET) $(OBJS) $(OBJS:.o=.d)



all: $(LINK_TARGET)
	@echo make all complete



$(LINK_TARGET): $(OBJS)
	gcc -o $@ $^

-include $(OBJS:.o=.d)

# pattern rule to compile .c files to .o and .d
%.o: %.c
	gcc -o $@ -c $<
	gcc -MM $(CFLAGS) $< > $*.d

	#things below are weird see: scottmcpeak.com/autodepend/autodepend.html
	@cp -f $*.d $*.d.tmp
	@sed -e 's/.*://' -e 's/\\$$//' < $*.d.tmp | fmt -1 | \
	  sed -e 's/^ *//' -e 's/$$/:/' >> $*.d
	@rm -f $*.d.tmp





clean:
	rm -f $(REBUILDABLES)
	@echo clean complete


