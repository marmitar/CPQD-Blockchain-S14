# # # # # # # # # # # # # #
# COMPILING THE USER APP  #

app_flags = cc.get_supported_arguments(
    '-fPIC',
    '-Wno-attributes',
    f'-I@SGX_SDK@/include',
    '-UEDEBUG',
    '-UDEBUG',
    checked: 'warn',
)

executable('app',
    files('App.cpp', 'TrustedLibrary/Libcxx.cpp'),
    untrusted_enclave,
    c_args: app_flags,
    cpp_args: app_flags,
    link_args: [f'-L@SGX_SDK@/lib64', '-lsgx_urts_sim', '-lpthread'],
)
