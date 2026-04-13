
#!/bin/bash
#Obtendo os argumentos de arquitetura e nível da API
ARCHITECTURE=$1
ANDROIDAPILEVEL=$2
if [ -z "$ARCHITECTURE" ] || [ -z "$ANDROIDAPILEVEL" ]; then
    echo "Uso: $0 <arquitetura> <nível da API>"
    echo "Exemplo: $0 arm64-v8a 30"
    exit 1
fi
adb root && 
make build arch=${ARCHITECTURE} api=${ANDROIDAPILEVEL} && adb push build/apkm-module-${ARCHITECTURE}-API${ANDROIDAPILEVEL}.zip /sdcard/apkm-module-installer.zip && adb shell "magisk --install-module /sdcard/apkm-module-installer.zip" && adb shell reboot
