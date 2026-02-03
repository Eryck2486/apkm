 # Define variables
arch ?= x86_64
api ?= 35
WORKDIR=$(PWD)
JSONREPO=https://github.com/nlohmann/json/
LIBZIPREPO=https://github.com/nih-at/libzip
CURLREPO=https://github.com/curl/curl
OPENSSLREPO=https://github.com/openssl/openssl
ANDROIDNDKLINK=https://dl.google.com/android/repository/android-ndk-r29-linux.zip?hl=pt-br

#Variavel para
SRCPATH=src
ANDROIDNDKROOT=$(WORKDIR)/tools/android-ndk-r29
TOOLCHAIN_RES = $(ANDROIDNDKROOT)/toolchains/llvm/prebuilt/linux-x86_64
SYSROOT = $(TOOLCHAIN_RES)/sysroot
INCLUDE_DIRS=\
	include \
	include/json/include/ \
	include/curl/include/ \
	include/openssl/include/ \
	include/libzip/

# Mapeamento para os nomes da Toolchain do NDK
CPPANDROIDBIN=$(ANDROIDNDKROOT)/toolchains/llvm/prebuilt/linux-x86_64/bin
LLVMAR=$(CPPANDROIDBIN)/llvm-ar
LLVMRANLIB=$(CPPANDROIDBIN)/llvm-ranlib
LLVMNM=$(CPPANDROIDBIN)/llvm-nm
LLVMSTRIP=$(CPPANDROIDBIN)/llvm-strip
LD=$(CPPANDROIDBIN)/ld.lld

OPENSSL_INST=$(WORKDIR)/include/openssl
LIBZIP_INST=$(WORKDIR)/include/libzip/build_output

ANDROIDAPILEVEL=$(api)
ARCHITECTURE=$(arch)
ifeq ($(ARCHITECTURE),arm64)
    NDK_ARCH=aarch64
    TOOLCHAINNAME=android
    OPENSSL_TARGET=android-arm64
    SPECIFIC_LIBS="-lssl"
else ifeq ($(ARCHITECTURE),x86)
    NDK_ARCH=i686
	TOOLCHAINNAME=android
	OPENSSL_TARGET=android-x86
    SPECIFIC_LIBS="-lssl"
else ifeq ($(ARCHITECTURE),arm)
    NDK_ARCH=armv7a
    TOOLCHAINNAME=androideabi
    # O OpenSSL para ARM 32-bit precisa desse alvo específico
    OPENSSL_TARGET=android-arm
    # IMPORTANTE: Forçar libatomic no ARMv7
    SPECIFIC_LIBS="-lssl"
else 
    NDK_ARCH=$(ARCHITECTURE)
	TOOLCHAINNAME=android
	OPENSSL_TARGET=android-x86_64
    SPECIFIC_LIBS="-lssl"
endif

CXXFLAGS = \
	-std=c++17 \
	-Wall $(INCLUDE_DIRS:%=-I%) \
	--sysroot=$(SYSROOT)

TARGET = build/apkm
SRCS = $(SRCPATH)/main.cpp \
	$(SRCPATH)/repository_manager.cpp \
	$(SRCPATH)/idiomas.cpp \
	$(SRCPATH)/apkm.cpp \
	$(SRCPATH)/gerenciador_pacotes.cpp

EXARGS=include/curl/lib/.libs/libcurl.a include/openssl/libssl.a include/openssl/libcrypto.a include/libzip/lib/libzip.a -static-libstdc++ -lz
RM = rm -f
ARGUMENTOSPADROES="-Wmacro-redefined"
CXX=$(CPPANDROIDBIN)/$(NDK_ARCH)-linux-$(TOOLCHAINNAME)$(ANDROIDAPILEVEL)-clang++
CC=$(CPPANDROIDBIN)/$(NDK_ARCH)-linux-$(TOOLCHAINNAME)$(ANDROIDAPILEVEL)-clang
PATH=$(ANDROIDNDKROOT)/toolchains/llvm/prebuilt/linux-x86_64/bin:/usr/bin

#Objetos a serem removidos na limpeza
OBJS= \
	include/curl/lib/.libs/libcurl.a \
	include/openssl/libssl.a \
	include/openssl/libcrypto.a \
	include/libzip/lib/libzip.a

build: $(SRCS) prepare
	$(MAKE) build_apkm

build_apkm: $(SRCS)
	$(CXX) $(CXXFLAGS) $(SRCS) $(EXARGS) -o $(TARGET);
	@if [ -e $(TARGET) ]; then \
		echo "Binário "$(TARGET)" compilado para arquitetura "$(ARCHITECTURE)" com sucesso."; \
	else echo "Falha ao criar o binário "$(TARGET)" para a arquitetura "$(ARCHITECTURE)"."; \
	fi;

prepare: clean
	@mkdir -p build;
	@mkdir -p tools;
	@mkdir -p include;
	#Baixa o NDK do android para cross-compile (recomenda-se sempre baixar a mais recente);
	@if [ ! -e tools/android-ndk-r29 ]; then \
		cd tools; \
		wget -O ndk.zip $(ANDROIDNDKLINK); \
		unzip ndk.zip; \
		$(RM) ndk.zip; \
		cd ..; \
	fi;

	#Baixa o nlohmann/json para processamento de Json;
	@if [ ! -e include/json ]; then \
		cd include; \
		if [ ! -e include/json ]; then git clone $(JSONREPO); fi; \
		cd ..; \
	fi;

	#Baixa e compila as bibliotecas openssl para conexão e verificação https através do curl;
	@if [ ! -e include/openssl/libssl.a ]; then \
		echo "Configurando OpenSSL\n"; \
		cd include; \
		if [ ! -e openssl ]; then git clone $(OPENSSLREPO); fi; \
		cd openssl; \
		PATH=$(PATH) \
		AR=$(LLVMAR) \
		CC="$(CC)" \
		NM=$(LLVMNM) \
		LD=$(LD) \
		STRIP=$(STRIP) \
		LDFLAGS="-L$(OPENSSL_INST)/" \
		CCFLAGS="-I$(OPENSSL_INST)/include $(ARGUMENTOSPADROES)" \
		RANLIB=$(LLVMRANLIB) \
		ANDROID_NDK_ROOT=$(ANDROIDNDKROOT) \
		./Configure $(OPENSSL_TARGET) no-shared \
			-D__ANDROID_API__=$(ANDROIDAPILEVEL) \
			--prefix=$(OPENSSL_INST) \
			--openssldir=$(OPENSSL_INST) && \
		make build_libs -j$(nproc) CCFLAGS=$(ARGUMENTOSPADROES); \
		cd ../../; \
		echo "\n OpenSSL Configurado"; \
	fi;

	#Baixa e compila o curld;
	@if [ ! -e include/curl/lib/.libs/libcurl.a ] && [ -e include/openssl/libssl.a ]; then \
		echo "Configurando Curl\n"; \
		cd include; \
		if [ ! -e curl ]; then git clone $(CURLREPO); fi; \
		cd curl; \
		echo "Configurando Curl..."; \
		autoreconf -fi && \
		PATH=$(PATH) \
		AR=$(LLVMAR) \
		CC=$(CC) \
		AS=$(CC) \
		NM=$(LLVMNM) \
		LD=$(LD) \
		STRIP=$(STRIP) \
		LDFLAGS="-L$(OPENSSL_INST)/" \
		CCFLAGS="-I$(OPENSSL_INST)/include $(ARGUMENTOSPADROES)" \
		RANLIB=$(LLVMRANLIB) \
		./configure --host=$(NDK_ARCH)-linux-$(TOOLCHAINNAME) \
			--enable-static \
			--disable-shared \
			--without-libpsl \
			--disable-ldap \
            --disable-ldaps \
			--with-openssl=$(OPENSSL_INST) \
			--prefix=$(WORKDIR)/include/curl/build_output \
			CPPFLAGS="-I$(OPENSSL_INST)/include -DANDROID -D__ANDROID_API__=$(ANDROIDAPILEVEL) $(ARGUMENTOSPADROES)" \
            LDFLAGS="-L$(OPENSSL_INST) -L$(SYSROOT)/usr/lib/$(NDK_ARCH)-linux-$(TOOLCHAINNAME)/$(ANDROIDAPILEVEL)" \
            LIBS="$(SPECIFIC_LIBS)" && \
		make -j$(nproc) CPPLAGS=$(ARGUMENTOSPADROES); \
		echo "\n Curl Configurado"; \
	else echo "Erro, falha ao compilar OpenSSL"; \
	fi;

	#Baixa e compila a biblioteca libzip para manipulação de arquivos zip/apk;
	@if [ ! -e include/libzip/lib/libzip.a ]; then \
		echo "Configurando libzip\n"; \
		cd include; \
		if [ ! -e libzip ]; then git clone $(LIBZIPREPO); fi; \
		cd libzip; \
		mkdir -p build && cd build; \
		PATH=$(PATH) \
		AR=$(LLVMAR) \
		CC="$(CC)" \
		NM=$(LLVMNM) \
		LD=$(LD) \
		STRIP=$(STRIP) \
		LDFLAGS="-L$(OPENSSL_INST)/" \
		CCFLAGS="-I$(OPENSSL_INST)/include $(ARGUMENTOSPADROES)" \
		RANLIB=$(LLVMRANLIB) \
		ANDROID_NDK_ROOT=$(ANDROIDNDKROOT) \
		cmake .. \
			-DCMAKE_TOOLCHAIN_FILE=$(ANDROIDNDKROOT)/build/cmake/android.toolchain.cmake \
			-DANDROID_ABI=$(ARCHITECTURE) \
			-DANDROID_PLATFORM=android-$(ANDROIDAPILEVEL) \
			-DBUILD_SHARED_LIBS=OFF \
			-DCMAKE_INSTALL_PREFIX=$(LIBZIP_INST) \
			-DENABLE_GNUTLS=OFF \
			-DENABLE_MBEDTLS=OFF \
			-DENABLE_BZIP2=OFF \
			-DENABLE_LZMA=OFF \
			-DENABLE_ZSTD=OFF \
			-DENABLE_OPENSSL=ON \
			-DBUILD_TOOLS=OFF \
			-DBUILD_EXAMPLES=OFF \
			-DBUILD_DOC=OFF \
			-DBUILD_REGRESS=OFF \
			-DBUILD_OSSFUZZ=OFF \
			-DOPENSSL_ROOT_DIR=$(WORKDIR)/include/openssl \
			-DOPENSSL_INCLUDE_DIR=$(WORKDIR)/include/openssl/include \
			-DOPENSSL_CRYPTO_LIBRARY=$(WORKDIR)/include/openssl/libcrypto.a \
			-DOPENSSL_SSL_LIBRARY=$(WORKDIR)/include/openssl/libssl.a \
			-DZLIB_LIBRARY=$(SYSROOT)/usr/lib/$(NDK_ARCH)-linux-$(TOOLCHAINNAME)/$(ANDROIDAPILEVEL)/libz.a \
			-DZLIB_INCLUDE_DIR=$(SYSROOT)/usr/include \
			-DCMAKE_INSTALL_LIBDIR=lib \
			-DCMAKE_INSTALL_INCLUDEDIR=include \
			-DCMAKE_INSTALL_BINDIR=bin; \
		cd ..; \
		make -j$(nproc); \
		cd ../../../; \
		echo "\n libzip Configurado"; \
	fi;


# Phony targets don't correspond to actual files
.PHONY: all clean

# Clean target: removes generated files
clean:
	$(RM) $(TARGET) $(OBJS)
