if (TARGET tinyusb_device)
    add_executable(SPI_test
            SPI_test.c
            )

    # pull in common dependencies
    target_link_libraries(SPI_test 
        pico_stdlib
        hardware_spi
    )

    # create map/bin/hex file etc.
    pico_add_extra_outputs(SPI_test)

    # add url via pico_set_program_url
    example_auto_set_url(SPI_test)

    pico_enable_stdio_usb(SPI_test 1)

elseif(PICO_ON_DEVICE)
        message(WARNING "not building the program becuase TinyUSB submodule is not initialized in the SDK")
endif()