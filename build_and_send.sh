
#!/bin/bash
adb root && 
make build_apkm && adb push build/apkm-module-installer.zip /sdcard && adb shell "magisk --install-module /sdcard/apkm-module-installer.zip" && adb shell reboot