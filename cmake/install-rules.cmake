install(
    TARGETS gameboy-emulator_exe
    RUNTIME COMPONENT gameboy-emulator_Runtime
)

if(PROJECT_IS_TOP_LEVEL)
  include(CPack)
endif()
