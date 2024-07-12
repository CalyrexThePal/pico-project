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
