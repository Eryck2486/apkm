[![Português Brasileiro](https://img.shields.io/badge/Language-PT--BR-green.svg)](README.pt-BR.md)
[![English](https://img.shields.io/badge/Language-EN-green.svg)](README.md)
[![Espanhol](https://img.shields.io/badge/Language-ES-green.svg)](README.es.md)

# apkm
Magisk/KernelSU module that offers an online package manager for Android with native access to repositories and AddOns to facilitate sideloading Apps from any Web source.

The goal of the project is to create an environment free from Google API restrictions. Use on Android AOSP is recommended, but it works perfectly on certified devices as long as they are running a Magisk-based root manager.

## How it works:

The manager is called apkm, which is an abbreviation for Android Package Manager.
The manager is a terminal program without a UI, meaning access to a terminal emulator or ADB is required. To have a UI, install APKM Store.

Type: 
```bash
apkm --help
```
in the terminal to get the available commands.

Addons or repository servers are required to have packages available.

It must be run with sudo or in a root terminal; otherwise, errors will occur.

## Notices:

This manager has not had its security effectiveness proven and does not guarantee system security; use it at your own risk.

Changes will be made to SELinux policies so that APKM Store (UI Assistant for non-technical users) can execute the program's sockets correctly without being blocked by context locks.

The official binary was compiled and tested on Android 16; other versions have not yet been tested.

## How to compile:

The compilation process can be done in three ways. Compiling the libraries with the binary (run without quotes), example for 64-bit arm: 
```bash 
make build arch=aarch64
``` 
generates the libraries, the apkm binary, and the magisk module.

The second way only works after the first one has been completed for the first time. It is useful when it is convenient to recompile only the apkm binary without recompiling the auxiliary libraries for quick tests. If the architecture changes, it is necessary to recompile the libraries again:

```bash
make build_apkm arch=aarch64
```
generates only the apkm binary and the magisk module.

The third way generates the magisk module directly:
```bash
make build_module
```
generates the magisk module.

## Libraries/toolchins used:
json: https://github.com/nlohmann/json
libzip: https://github.com/nih-at/libzip
libcurl https://github.com/curl/curl
openssl https://github.com/openssl/openssl
Android NDK: https://dl.google.com/android/repository/android-ndk-r29-linux.zip