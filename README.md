# apkm
Módulo Magisk/KernelSU que oferece um gerenciador de pacotes online para Android AOSP com acesso a repositórios e AddOns nativamente para facilitar sideloading de Apps de quaisquer fontes da Web

O objetivo do projeto é criar um ambiente livre das restrições da API do Google, recomenda-se o uso no Android AOSP, mas funciona perfeitamente em dispositivos certificados desde que ele esteja execultando um gerenciador root baseado no Magisk root.

## Como funciona:

O gerenciador se chama apkm que é uma abreviação de Android Package Manager
O gerenciador é um programa de terminal sem UI, ou seja, é necessário o acesso a um emulador de terminal ou acesso ao ADB, para ter uma UI instale o APKM Store.

Digite: 
```bash
apkm --help
```
no terminal para obter os comandos disponíveis.

Addons ou servidores de repositório são necessários para ter pacotes disponíveis.

Deve ser execultado com sudo ou em terminal root, caso contrário ocorrerão erros.
    

## Avisos:

Esse gerenciador não teve a eficácia da segurança comprovada e não garante a segurança do sistema, use por sua conta e risco

Serão feitas alterações nas políticas do SELinux para que o APKM Store (Assistente de UI para pessoas leigas) consiga execultar os sockets do programa corretamente sem ser bloqueado por trava de contexto.

O binário oficial foi compilado e testado no Android 16, outras versões ainda não foram testadas, versões de 32 bits não são suportadas oficialmente.


## Como compilar:

O processo de compilação tem três formas de ser feita, a compilação das bibliotecas com o binário (Execultar sem aspas) exemplo para arm de 64 bits: 
```bash 
make build arch=aarch64
``` 
gera as bibliotecas, binário do apkm e o módulo magisk

A segunda forma só funciona após a primeira ser concluida pela primeira vez, ela é util quando é conveniente recompilar apenas o binário do apkm sem recompilar as bibliotecas auxiliares para testes rápidos, se a arquitetura mudar é necessário recompilar as bibliotecas novamente:

```bash
make build_apkm arch=aarch64
```
gera apenas o binário do apkm e o módulo magisk

A terceira forma gera o módulo magisk diretamente:
```bash
make build_module
```
gera o módulo magisk


## Bibliotecas/toolchins utilizadas:
    json: https://github.com/nlohmann/json/
    libzip: https://github.com/nih-at/libzip
    libcurl https://github.com/curl/curl
    openssl https://github.com/openssl/openssl
    Android NDK: https://dl.google.com/android/repository/android-ndk-r29-linux.zip?hl=pt-br