add_executable(adc_trap
        adc_trap.c
        )

# pull in common dependencies
target_link_libraries(skeleton pico_stdlib hardware_adc hardware_dma hardware_irq hardware_timer)

# create map/bin/hex file etc.
pico_add_extra_outputs(adc_trap)

# add url via pico_set_program_url
example_auto_set_url(adc_trap)
