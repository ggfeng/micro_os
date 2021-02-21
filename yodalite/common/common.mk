OBJ := $(SRC:.c=.o)

ALL:$(MODULE) install

$(MODULE):$(OBJ)
	$(AR) -rcs $@ $^

install:
	@cp $(MODULE) $(LIB_DIR)

%.o: %.c
	$(CC) -o $@ $(CFLAGS) -c $^

.PHONY: clean

clean:
	@rm -rf $(OBJ) $@ $(MODULE)
