# apkm
Módulo Magisk/KernelSU que oferece um gerenciador de pacotes online para Android AOSP com acesso a repositórios e AddOns nativamente para facilitar sideloading de Apps de quaisquer fontes da Web

O objetivo do projeto é criar um ambiente livre das restrições da API do Google, recomenda-se o uso no Android AOSP, mas funciona perfeitamente em dispositivos certificados desde que ele esteja execultando um gerenciador root baseado no Magisk root.

Como funciona:
    O gerenciador se chama apkm que é uma abreviação de Android Package Manager
    O gerenciador é um programa de terminal sem UI, ou seja, é necessário o acesso a um emulador de terminal ou acesso ao ADB, para ter uma UI instale o APKM Store.
    
    Digite "apkm --help" (Sem aspas) no terminal para obter os comandos disponíveis.
    
    Addons ou servidores de repositório são necessários para ter pacotes disponíveis.
    
    Deve ser execultado com sudo ou em terminal root, caso contrário ocorrerão erros.
    

Avisos:
    Esse gerenciador não teve a eficácia da segurança comprovada e não garante a segurança do sistema, use por sua conta e risco
    
    Serão feitas alterações nas políticas do SELinux para que o APKM Store (Assistente de UI para pessoas leigas) consiga execultar os sockets do programa corretamente sem ser bloqueado por trava de contexto.

    O binário oficial foi compilado e testado no Android 16, outras versões ainda não foram testadas, versões de 32 bits não são suportadas oficialmente.

