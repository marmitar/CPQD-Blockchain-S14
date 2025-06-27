# # # # # # # # # # # # # #
# COMPILING THE USER APP  #

executable('app',
    files('App.cpp', 'TrustedLibrary/Libcxx.cpp'),
    untrusted_enclave,
    c_args: SGX_COMMON_CFLAGS + APP_FLAGS,
    cpp_args: SGX_COMMON_CXXFLAGS + APP_FLAGS,
    link_args: ['-L/opt/intel/sgxsdk/lib64', '-lsgx_urts_sim', '-lpthread'],
)
