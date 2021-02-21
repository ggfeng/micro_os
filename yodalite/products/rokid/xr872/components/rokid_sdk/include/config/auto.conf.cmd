deps_config := \
	products/Kconfig \
	osal/Kconfig \
	lib/mem_ext/Kconfig \
	lib/nettool/Kconfig \
	lib/fatfs/Kconfig \
	lib/FreeRTOS/Kconfig \
	lib/libc/Kconfig \
	lib/Kconfig \
	hardware/driver/Kconfig \
	hardware/pal/Kconfig \
	hardware/modules/Kconfig \
	hardware/Kconfig \
	hapi/factory/Kconfig \
	hapi/Kconfig \
	app/Kconfig \
	aial/Kconfig \
	Kconfig

include/config/auto.conf: \
	$(deps_config)


$(deps_config): ;
