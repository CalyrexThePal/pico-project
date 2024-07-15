if (TARGET tinyusb_device)
        add_executable(adc_multi
                adc_multi.c
                )

        # pull in common dependencies
        target_link_libraries(adc_multi 
                pico_stdlib 
                hardware_adc
                hardware_dma 
                hardware_irq 
                hardware_timer 
                hardware_uart 
                hardware_i2c 
                hardware_spi
        )

        # create map/bin/hex file etc.
        pico_add_extra_outputs(adc_multi)

        # add url via pico_set_program_url
        example_auto_set_url(adc_multi)


        pico_enable_stdio_usb(adc_multi 1) # enable USB
        pico_enable_stdio_uart(adc_multi 0) # disaable UART

elseif(PICO_ON_DEVICE)
        message(WARNING "not building the program becuase TinyUSB submodule is not initialized in the SDK")
endif()

