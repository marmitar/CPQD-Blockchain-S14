# # # # # # # # # # # # #
# COMPILING THE ENCLAVE #

ENCLAVE_INCLUDES = [
    f'-I@SGX_SDK@/include',
    f'-I@SGX_SDK@/include/libcxx',
    f'-I@SGX_SDK@/include/tlibc',
]
ENCLAVE_FLAGS = [
    '-nostdinc',
    '-fvisibility=hidden',
    '-fpie',
    '-fstack-protector',
    '-fno-builtin-printf',
] + ENCLAVE_INCLUDES + [
    '-nostdinc++',
]
ENCLAVE_LINK_FLAGS = [
    '-Wl,-z,relro,-z,now,-z,noexecstack',
    '-Wl,--no-undefined',
    '-nostdlib', '-nodefaultlibs', '-nostartfiles', f'-L@SGX_SDK@/lib64',
    '-Wl,--whole-archive',
    '-lsgx_trts_sim',
    '-Wl,--no-whole-archive',
    '-Wl,--start-group',
    '-lsgx_tstdc -lsgx_tcxx -lsgx_tcrypto -lsgx_trts_sim',
    '-Wl,--end-group',
    '-Wl,-Bstatic',
    '-Wl,-Bsymbolic',
    '-Wl,--no-undefined',
    '-Wl,-pie,-eenclave_entry',
    '-Wl,--export-dynamic',
    '-Wl,--defsym,__ImageBase=0',
    '-Wl,--version-script=@0@/Enclave_debug.lds'.format(meson.current_source_dir()),
]

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
    c_args: ['-nostdinc', '-fvisibility=hidden', '-fpie', '-fstack-protector', '-fno-builtin-printf']
        + ENCLAVE_INCLUDES,
    cpp_args: ENCLAVE_FLAGS,
    link_args: ENCLAVE_LINK_FLAGS ,
    name_prefix: '',
)

custom_target('signed-enclave',
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
