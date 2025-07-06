# Intel SGX - Hello World

Sample project using [Intel® Software Guard Extensions (SGX)](https://www.intel.com/content/www/us/en/developer/tools/software-guard-extensions/overview.html).

## Build

```sh
meson setup build
meson compile -C build
```

## Run

```sh
source /opt/intel/sgxsdk/environment
build/App/app build/Enclave/enclave.signed.so
```
