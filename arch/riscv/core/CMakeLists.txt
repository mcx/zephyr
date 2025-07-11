# SPDX-License-Identifier: Apache-2.0

zephyr_library()

zephyr_library_sources(
  cpu_idle.c
  fatal.c
  irq_manage.c
  prep_c.c
  reboot.c
  reset.S
  switch.S
  thread.c
)

if ((CONFIG_MP_MAX_NUM_CPUS GREATER 1) OR (CONFIG_SMP))
  zephyr_library_sources(smp.c)
endif ()

if (CONFIG_SMP)
  zephyr_library_sources(ipi.c)

  zephyr_library_sources_ifdef(CONFIG_RISCV_SMP_IPI_CLINT ipi_clint.c)
endif()

zephyr_library_sources_ifdef(CONFIG_FPU_SHARING fpu.c fpu.S)
zephyr_library_sources_ifdef(CONFIG_DEBUG_COREDUMP coredump.c)
zephyr_library_sources_ifdef(CONFIG_IRQ_OFFLOAD irq_offload.c)
zephyr_library_sources_ifdef(CONFIG_GEN_SW_ISR_TABLE isr.S)
zephyr_library_sources_ifdef(CONFIG_RISCV_PMP pmp.c pmp.S)
zephyr_library_sources_ifdef(CONFIG_THREAD_LOCAL_STORAGE tls.c)
zephyr_library_sources_ifdef(CONFIG_USERSPACE userspace.S)
zephyr_library_sources_ifdef(CONFIG_SEMIHOST semihost.c)
zephyr_library_sources_ifdef(CONFIG_ARCH_STACKWALK stacktrace.c)
zephyr_linker_sources(ROM_START SORT_KEY 0x0vectors vector_table.ld)
zephyr_library_sources_ifdef(CONFIG_LLEXT elf.c)

if(CONFIG_GEN_SW_ISR_TABLE)
  if(CONFIG_DYNAMIC_INTERRUPTS)
    zephyr_linker_sources(RWDATA swi_tables.ld)
  else()
    zephyr_linker_sources(RODATA swi_tables.ld)
  endif()
endif()
