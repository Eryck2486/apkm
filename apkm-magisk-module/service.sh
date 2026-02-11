#!/system/bin/sh
MODDIR=${0%/*}

# Aguarda o boot completar para garantir que o SystemServer esteja pronto
while [ "$(getprop sys.boot_completed)" != "1" ]; do
  sleep 2
done

# Define o CLASSPATH para o seu JAR
export CLASSPATH=$MODDIR/helper.jar

# Executa em background (&) usando o app_process
# O nome do processo aparecerá como "apkm_helper" no top/ps
nohup app_process /system/bin HelperService > /dev/null 2>&1 &