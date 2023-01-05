if (TARGET tinyusb_device)
    add_executable(fancontroller
            fancontroller.c ./ssd1306.c
            )

    # pull in common dependencies
    target_link_libraries(fancontroller pico_stdlib 
	pico_cyw43_arch_none hardware_i2c)
	
	set(PICO_BOARD pico_w)

    # enable usb output, disable uart output
    pico_enable_stdio_usb(fancontroller 1)
    pico_enable_stdio_uart(fancontroller 0)

    # create map/bin/hex/uf2 file etc.
    pico_add_extra_outputs(fancontroller)

    # add url via pico_set_program_url
    example_auto_set_url(fancontroller)
elseif(PICO_ON_DEVICE)
    message(WARNING "not building fancontroller because TinyUSB submodule is not initialized in the SDK")
endif()
