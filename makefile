LINK_TARGET = a4.out

INC = \
 src


INC_PARAMS= -I. $(foreach d, $(INC), -I$d)

CFLAGS = $(INC_PARAMS)

VPATH = .$(foreach d, $(INC),:$d)

# OBJS =  \
#  A4main.o \
#  altstring.o \
#  AUDIO.o \
#  conversions.o \
#  LCD_Control.o \
#  opencores_i2c.o \
#  SD_functions.o \
#  terminalFunctions.o \
#  terminalParse.o \
#  wavPlay.o

OBJS = \
test.o \
altstring.o \
sub.o 



REBUILDABLES = $(LINK_TARGET) $(OBJS) $(OBJS:.o=.d)



all: $(LINK_TARGET)
	@echo make all complete



$(LINK_TARGET): $(OBJS)
	gcc -o $@ $^

-include $(OBJS:.o=.d)

# pattern rule to compile .c files to .o and .d
# things (@ commands) below are weird see: scottmcpeak.com/autodepend/autodepend.html
%.o: %.c
	gcc $(CFLAGS) -o $@ -c $<
	gcc -MM $(CFLAGS) $< > $*.d

	
	@cp -f $*.d $*.d.tmp
	@sed -e 's/.*://' -e 's/\\$$//' < $*.d.tmp | fmt -1 | \
	  sed -e 's/^ *//' -e 's/$$/:/' >> $*.d
	@rm -f $*.d.tmp





clean:
	rm -f $(REBUILDABLES)
	@echo clean complete


