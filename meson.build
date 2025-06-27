project('hello-world-intel-sgx', 'c', 'cpp',
    version: '0.1.0',
    license: 'BSD-3-Clause',
    meson_version: '>= 1.4.0',
    subproject_dir: 'external',
)

warning_level = get_option('warning_level') == 'everything' ? 5 : get_option('warning_level').to_int()
optimizations_enabled = get_option('optimization') in ['2', '3', 's', 'z']
debugging_enabled = get_option('debug')

cc = meson.get_compiler('c')
cxx = meson.get_compiler('cpp')

# # # # # # # # # # # #
# SGX TOOLS AND LIBS  #

SGX_SDK = '/opt/intel/sgxsdk'
SGX_PATH = [SGX_SDK / 'bin', SGX_SDK / 'bin/x64']
SGX_INCLUDE = SGX_SDK / 'include'
SGX_PKGCONFIG = SGX_SDK / 'pkgconfig'
SGX_LDLIBRARY = SGX_SDK / 'sdk_libs'

sgx_edger8r = find_program('sgx_edger8r', dirs: SGX_PATH)
sgx_sign = find_program('sgx_sign', dirs: SGX_PATH)

# # # # # # # # # #
# COMPILER FLAGS  #

warnings = []
cpp_warnings = []
# Safety warnings
if warning_level >= 1
    warnings += cc.get_supported_arguments(
        '-Wconversion',
        '-Wsign-conversion',
        '-Wformat',
        '-Wformat=2',
        '-Wformat-signedness',
        '-Winit-self',
        '-Wpointer-arith',
        '-Wreturn-type',
        '-Waddress',
        '-Wsequence-point',
        '-Wformat-security',
        '-Wmissing-include-dirs',
        '-Wfloat-equal',
        '-Wundef',
        '-Wshadow',
        '-Wcast-align',
        '-Wcast-qual',
        '-Wconversion',
        '-Wredundant-decls',
        '-Wjump-misses-init',
        '-Wstrict-prototypes',
        '-Wunsuffixed-float-constants',
        cc.get_supported_arguments('-Wformat-truncation=2').get(0, '-Wformat-truncation'),
        cc.get_supported_arguments('-Wformat-overflow=2').get(0, '-Wformat-overflow'),
        checked: 'warn',
    ) + cc.get_supported_arguments(
        '-Wstringop-overflow=4', # GCC only
        '-Wno-nullability-extension', # Clang only
        checked: 'off',
    )

    cpp_warnings += cxx.get_supported_arguments(
        '-Wnon-virtual-dtor',
        checked: 'warn',
    )
endif
# Extra warnings
if warning_level >= 2
    warnings += cc.get_supported_arguments(
        '-Wunused-result',
        checked: 'warn',
    )
endif
# Pedantic warnings
if warning_level >= 3
    warnings += cc.get_supported_arguments(
        optimizations_enabled ? '-Winline' : [],
        checked: 'warn',
    ) + cc.get_supported_arguments(
        optimizations_enabled ? '-Rpass=.*' : [], # Clang only
        optimizations_enabled ? '-Rpass-missed=.*' : [], # Clang only
        checked: 'off',
    )
endif
# Disable some bugprone GCC warnings
if cc.get_id() == 'gcc'
    warnings += cc.get_supported_arguments(
        '-Wno-abi',
        '-Wno-nonnull-compare',
        '-Wno-unused-function',
        checked: 'warn'
    )
endif

optimizations = []
if optimizations_enabled
    optimizations += cc.get_supported_arguments(
        not meson.is_cross_build() ? '-march=native' : [],
        not meson.is_cross_build() ? '-mtune=native' : [],
        not get_option('b_staticpic') ? '-fno-pic' : [],
        not get_option('b_pie') ? '-fno-pie' : [],
        not get_option('b_pie') ? '-fno-plt' : [],
        '-fdata-sections',
        '-ffunction-sections',
        '-fno-exceptions',
        '-fno-math-errno',
        '-fno-signaling-nans',
        '-fno-trapping-math',
        checked: 'warn',
    ) + cc.get_supported_arguments(
        get_option('b_lto_mode') == 'default' ? '-flto=full' : [], # Clang only
        '-fallow-store-data-races', # GCC only
        '-fwhole-program', # GCC only
        checked: 'off',
    )
endif

general_codegen = cc.get_supported_arguments(
    '-pipe',
    '-fvisibility=hidden',
    debugging_enabled ? '-fstrict-overflow' : '-fno-strict-overflow',
    # _FORTIFY_SOURCE=2+ breaks static analysis on some stdlib functions
    '-D_FORTIFY_SOURCE=@0@'.format(debugging_enabled and cc.get_id() == 'gcc' ? 3 : 1),
    checked: 'warn',
)

debugging = []
if debugging_enabled
    debugging += cc.get_supported_arguments(
        '-DDEBUG',
        '-ggdb',
        '-g3',
        '-fstack-clash-protection',
        '-fcf-protection',
        '-ftrapv',
        '-fno-omit-frame-pointer',
        '-mno-omit-leaf-frame-pointer',
        checked: 'warn',
    )
endif

add_global_arguments(
    warnings,
    optimizations,
    general_codegen,
    debugging,
    language: 'c',
)

add_global_arguments(
    warnings,
    cpp_warnings,
    optimizations,
    general_codegen,
    debugging,
    language: 'cpp',
)

linker_options = cc.get_supported_link_arguments(
    optimizations_enabled ? '-Wl,-O1' : [],
    optimizations_enabled and not get_option('b_pie') ? '-no-pie' : [],
    '-Wl,--gc-sections',
    '-Wl,--sort-common',
    '-Wl,--as-needed',
    '-Wl,-z,relro',
    '-Wl,-z,now',
    '-Wl,-z,pack-relative-relocs'
)

add_global_link_arguments(
    linker_options,
    language: 'c',
)

# # # # # #
# TARGETS #

subdir('Enclave')
subdir('App')
