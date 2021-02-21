ALL:suddir 

suddir: CHECK_DIR
	$(foreach file,$(obj-y),make -C $(file) || exit "$$?";)

CHECK_DIR:
#	$(Q)mkdir -p "$(obj-y)"
clean:
	$(foreach file,$(obj-y),make -C $(file) clean;)

