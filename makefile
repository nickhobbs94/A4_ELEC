LINK_TARGET = a4.out

OBJS =  \
 test.o \
 conversions.o \
 altstring.o

SUBDIR = 

REBUILDABLES = $(OBJS) $(LINK_TARGET)



all: $(LINK_TARGET)



$(LINK_TARGET): $(OBJS)
	gcc -o $@ $^



# pattern rule to compile .c files to .o
%.o: %.c
	gcc -o $@ -c $<

# dependancy rule generator

%.d: %.c
	gcc -M $< > $@

include $(OBJS:.o=.d)


clean:
	rm -f $(REBUILDABLES)
	rm *.d


