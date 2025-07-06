# # # # # # # # # # # # # #
# COMPILING THE USER APP  #

app = executable('app',
    files('App.cpp', 'TrustedLibrary/Libcxx.cpp'),
    untrusted_enclave,
    dependencies: [sgx_urts],
    link_args: ['-Wl,-z,pack-relative-relocs'],
)

custom_target('disassembly',
  input: app,
  output: 'App.s',
  command: ['objdump', '-d', '-S', '--no-show-raw-insn', '--no-addresses', '@INPUT@'],
  capture: true,
  build_by_default: true
)
