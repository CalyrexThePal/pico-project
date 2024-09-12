if (TARGET tinyusb_device)
        add_executable(adc_A
                adc_A.c
                )

        # pull in common dependencies
        target_link_libraries(adc_A 
                pico_stdlib 
                hardware_adc
                hardware_irq 
                hardware_timer 
                hardware_uart 
                hardware_spi
        )

        # create map/bin/hex file etc.
        pico_add_extra_outputs(adc_A)

        # add url via pico_set_program_url
        example_auto_set_url(adc_A)


        pico_enable_stdio_usb(adc_A 1) # enable USB
        pico_enable_stdio_uart(adc_A 0) # disaable UART

elseif(PICO_ON_DEVICE)
        message(WARNING "not building the program becuase TinyUSB submodule is not initialized in the SDK")
endif()

