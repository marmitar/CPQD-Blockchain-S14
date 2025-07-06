# # # # # # # # # # # # # #
# COMPILING THE USER APP  #

app = executable('app',
    files('App.cpp', 'TrustedLibrary/Libcxx.cpp'),
    untrusted_enclave,
    dependencies: [sgx_urts],
    link_args: ['-Wl,-z,pack-relative-relocs'],
)
