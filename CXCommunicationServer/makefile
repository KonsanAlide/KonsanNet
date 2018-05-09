CC = gcc  
RM = rm  
  
CFLAGS += -D _YUQIANG  
TARGETS := myapp  
all:$(TARGETS)  
  
$(TARGETS):main.c  
$(CC) $(CFLAGS) $^ -o $@  
  
clean:  
-$(RM) -f *.o  
-$(RM) -f $(TARGETS)  