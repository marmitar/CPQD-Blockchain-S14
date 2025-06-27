# # # # # # # # # # # # #
# COMPILING THE ENCLAVE #

trusted_enclave = custom_target('trusted-enclave',
    command: [
        sgx_edger8r,
        '--search-path', meson.current_source_dir(),
        '--search-path', SGX_INCLUDE,
        '--trusted',
        '@INPUT@'
    ],
    input: files('Enclave.edl'),
    output: ['Enclave_t.c', 'Enclave_t.h'],
)

untrusted_enclave = custom_target('untrusted-enclave',
    command: [
        sgx_edger8r,
        '--search-path', meson.current_source_dir(),
        '--search-path', SGX_INCLUDE,
        '--untrusted',
        '@INPUT@'
    ],
    input: files('Enclave.edl'),
    output: ['Enclave_u.c', 'Enclave_u.h'],
)

enclave = shared_library('enclave',
    files('Enclave.cpp', 'TrustedLibrary/Libcxx.cpp'),
    trusted_enclave,
    c_args: ['-m64', '-O2', '-Wall', '-Wextra', '-Winit-self', '-Wpointer-arith', '-Wreturn-type', '-Waddress', '-Wsequence-point', '-Wformat-security', '-Wmissing-include-dirs', '-Wfloat-equal', '-Wundef', '-Wshadow', '-Wcast-align', '-Wcast-qual', '-Wconversion', '-Wredundant-decls', '-Wjump-misses-init', '-Wstrict-prototypes', '-Wunsuffixed-float-constants', '-nostdinc', '-fvisibility=hidden', '-fpie', '-fstack-protector', '-fno-builtin-printf']
        + ENCLAVE_INCLUDES,
    cpp_args: SGX_COMMON_CXXFLAGS + ENCLAVE_FLAGS,
    link_args: ENCLAVE_LINK_FLAGS ,
    name_prefix: '',
)

signed_enclave = custom_target('signed-enclave',
    command: [
        sgx_sign, 'sign',
        '-key', files('Enclave_private_test.pem'),
        '-config', files('Enclave.config.xml'),
        '-enclave', '@INPUT@',
        '-out', '@OUTPUT@',
    ],
    input: enclave,
    output: 'enclave.signed.so',
)
