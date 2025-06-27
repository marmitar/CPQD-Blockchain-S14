# # # # # # # # # # # # # #
# COMPILING THE USER APP  #

threads = dependency('threads')

executable('app',
    files('App.cpp', 'TrustedLibrary/Libcxx.cpp'),
    untrusted_enclave,
    dependencies: [sgx_urts, threads],
    link_args: ['-Wl,-z,pack-relative-relocs'],
)
