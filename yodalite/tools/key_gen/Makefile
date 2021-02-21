.PHONY:all clean
CFILES=keygen.c
CFLAGS=-I../include
DEPLIBS=-lcrypto
APP=keygen
LOCAL_CC=gcc
#PATH := $(shell pwd) : $(PATH)

all:$(APP)

$(APP):$(CFILES)
	$(LOCAL_CC) $(CFLAGS) -o $@ $^ $(DEPLIBS)
#	./keygen $(PRODUCT_BUILD_DIR)/public_key/rsa_public_key.pem rsa_public_key.c
	./keygen rsa_public_key.pem rsa_public_key.h
	mkdir -p $(PRJ_DIR)/include/lib/librsa
	mv rsa_public_key.h $(PRJ_DIR)/include/lib/librsa/

clean:
	rm -f *.o $(APP)
