# # # # # # # # # # # #
# SGX TOOLS AND LIBS  #

SGX_SDK = '/opt/intel/sgxsdk'
SGX_PATH = [SGX_SDK / 'bin', SGX_SDK / 'bin/x64']
SGX_INCLUDE = SGX_SDK / 'include'
SGX_PKGCONFIG = SGX_SDK / 'pkgconfig'
SGX_LDLIBRARY = SGX_SDK / 'sdk_libs'

sgx_edger8r = find_program('sgx_edger8r', dirs: SGX_PATH)
sgx_sign = find_program('sgx_sign', dirs: SGX_PATH)

sgx = declare_dependency(
    compile_args: [
        '-nostdinc',
        '-fvisibility=hidden',
        '-fpie',
        '-fstack-protector',
        '-fno-builtin-printf',
        '-nostdinc++',
    ],
    include_directories: [
        SGX_INCLUDE,
        SGX_INCLUDE / 'libcxx',
        SGX_INCLUDE / 'tlibc',
    ],
    link_args: [
        '-Wl,-z,relro,-z,now,-z,noexecstack',
        '-Wl,--no-undefined',
        '-nostdlib', '-nodefaultlibs', '-nostartfiles', f'-L@SGX_SDK@/lib64',
        '-Wl,--whole-archive',
        '-lsgx_trts_sim',
        '-Wl,--no-whole-archive',
        '-Wl,--start-group',
        '-lsgx_tstdc',
        '-lsgx_tcxx',
        '-lsgx_tcrypto',
        '-lsgx_trts_sim',
        '-Wl,--end-group',
        '-Wl,-Bstatic',
        '-Wl,-Bsymbolic',
        '-Wl,--no-undefined',
        '-Wl,-pie,-eenclave_entry',
        '-Wl,--export-dynamic',
        '-Wl,--defsym,__ImageBase=0',
    ]
)

# # # # # # # # # # # # #
# COMPILING THE ENCLAVE #

trusted_enclave = custom_target('trusted-enclave',
    command: [
        sgx_edger8r,
        '--search-path', '@CURRENT_SOURCE_DIR@',
        '--search-path', SGX_INCLUDE,
        '--trusted',
        '--trusted-dir', '@OUTDIR@',
        '@INPUT@'
    ],
    input: files('Enclave.edl'),
    output: ['Enclave_t.c', 'Enclave_t.h'],
)

untrusted_enclave = custom_target('untrusted-enclave',
    command: [
        sgx_edger8r,
        '--search-path', '@CURRENT_SOURCE_DIR@',
        '--search-path', SGX_INCLUDE,
        '--untrusted',
        '--untrusted-dir', '@OUTDIR@',
        '@INPUT@'
    ],
    input: files('Enclave.edl'),
    output: ['Enclave_u.c', 'Enclave_u.h'],
)

enclave = shared_library('enclave',
    files('Enclave.cpp', 'TrustedLibrary/Libcxx.cpp'),
    trusted_enclave,
    link_args: [
        '-Wl,--version-script=@0@/Enclave_debug.lds'.format(meson.current_source_dir()),
    ],
    dependencies: [sgx],
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
