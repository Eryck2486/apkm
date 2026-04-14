[![Português Brasileiro](https://img.shields.io/badge/Language-PT--BR-green.svg)](README.pt-BR.md)
[![English](https://img.shields.io/badge/Language-EN-green.svg)](README.md)
[![Espanhol](https://img.shields.io/badge/Language-ES-green.svg)](README.es.md)

# apkm
Módulo Magisk/KernelSU que ofrece un gestor de paquetes en línea para Android con acceso a repositorios y AddOns de forma nativa para facilitar el sideloading de Apps desde cualquier fuente de la Web.

El objetivo del proyecto es crear un entorno libre de las restricciones de la API de Google. Se recomienda su uso en Android AOSP, pero funciona perfectamente en dispositivos certificados siempre que ejecuten un gestor root basado en Magisk.

# Screenshots:
<img width="2560" height="1600" alt="image" src="https://github.com/user-attachments/assets/1e00e32c-c14b-4e17-ae84-85a59f03fb67" />
<img width="2560" height="1600" alt="image" src="https://github.com/user-attachments/assets/e28994b1-b81a-4361-b31c-da0684be1a98" />
<img width="2560" height="1600" alt="image" src="https://github.com/user-attachments/assets/98f0a6de-426b-4c0a-9964-ed8cfcea7557" />
<img width="2560" height="1600" alt="image" src="https://github.com/user-attachments/assets/71b51a42-d1e3-4b6e-866b-a11dfb0f6c13" />

## Cómo funciona:

El gestor se llama apkm, que es una abreviatura de Android Package Manager.
El gestor es un programa de terminal sin interfaz de usuario (UI), es decir, es necesario acceder a un emulador de terminal o acceso a ADB. Para tener una interfaz de usuario, instale APKM Store.

Escriba: 
```bash
apkm --help
```
en el terminal para obtener los comandos disponibles.

Se requieren Addons o servidores de repositorios para tener paquetes disponibles.

Debe ejecutarse con sudo o en un terminal root; de lo contrario, se producirán errores.

## Avisos:

Este gestor no ha demostrado su eficacia en cuanto a seguridad y no garantiza la seguridad del sistema, úselo bajo su propio riesgo.

Se realizarán cambios en las políticas de SELinux para que APKM Store (Asistente de UI para usuarios no técnicos) pueda ejecutar los sockets del programa correctamente sin ser bloqueado por bloqueos de contexto.

El binario oficial fue compilado y probado en Android 16; otras versiones aún no han sido probadas.

## Cómo compilar:

El proceso de compilación se puede realizar de tres formas. La compilación de las bibliotecas con el binario (ejecutar sin comillas), ejemplo para arm de 64 bits: 
```bash 
make build arch=aarch64
``` 
genera las bibliotecas, el binario de apkm y el módulo magisk.

La segunda forma solo funciona después de que la primera se haya completado por primera vez. Es útil cuando es conveniente recompilar solo el binario de apkm sin recompilar las bibliotecas auxiliares para pruebas rápidas. Si la arquitectura cambia, es necesario recompilar las bibliotecas nuevamente:

```bash
make build_apkm arch=aarch64
```
genera solo el binario de apkm y el módulo magisk.make build_module
make build_modulemake build_module

## Bibliotecas/toolchins utilizadas:
json: https://github.com/nlohmann/json
libzip: https://github.com/nih-at/libzip
libcurl https://github.com/curl/curl
openssl https://github.com/openssl/openssl
Android NDK: https://dl.google.com/android/repository/android-ndk-r29-linux.zip
