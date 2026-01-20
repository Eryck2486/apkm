 # Define variables
	
WORKDIR=$(PWD)
JSONREPO=https://github.com/nlohmann/json/
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
	include/openssl/include/

# Mapeamento para os nomes da Toolchain do NDK
CPPANDROIDBIN=$(ANDROIDNDKROOT)/toolchains/llvm/prebuilt/linux-x86_64/bin
LLVMAR=$(ANDROIDNDKROOT)/toolchains/llvm/prebuilt/linux-x86_64/bin/llvm-ar
LLVMRAMLIB=$(ANDROIDNDKROOT)/toolchains/llvm/prebuilt/linux-x86_64/bin/llvm-ramlib
LLVMNM=$(ANDROIDNDKROOT)/toolchains/llvm/prebuilt/linux-x86_64/bin/llvm-nm
LD=$(ANDROIDNDKROOT)/toolchains/llvm/prebuilt/linux-x86_64/bin/ld.lld
OPENSSL_INST=$(WORKDIR)/include/openssl


ANDROIDAPILEVEL=35
ARCHITECTURE=arm64
ifeq ($(ARCHITECTURE),arm64)
    NDK_ARCH=aarch64
else ifeq ($(ARCHITECTURE),arm)
    NDK_ARCH=armv7a
else
    NDK_ARCH=$(ARCHITECTURE)
endif

CXXFLAGS = \
	-std=c++17 \
	-Wall $(INCLUDE_DIRS:%=-I%) \
	--sysroot=$(SYSROOT)

TARGET = build/apkm
SRCS = $(SRCPATH)/main.cpp \
	$(SRCPATH)/repository-manager.cpp \
	$(SRCPATH)/idiomas.cpp \
	$(SRCPATH)/utilitarios.cpp

EXARGS=include/curl/lib/.libs/libcurl.a include/openssl/libssl.a include/openssl/libcrypto.a -static-libstdc++ -lz
RM = rm -f

CXX=$(CPPANDROIDBIN)/$(NDK_ARCH)-linux-android$(ANDROIDAPILEVEL)-clang++
CC=$(CPPANDROIDBIN)/$(NDK_ARCH)-linux-android$(ANDROIDAPILEVEL)-clang
PATH=$(ANDROIDNDKROOT)/toolchains/llvm/prebuilt/linux-x86_64/bin:/usr/bin

build: $(SRCS) prepare
	$(CXX) $(CXXFLAGS) $(SRCS) $(EXARGS) -o $(TARGET)


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
		git clone $(JSONREPO); \
		cd ..; \
	fi;

	#Baixa e compila as bibliotecas openssl para conexão e verificação https através do curl;
	@if [ ! -e include/openssl/libssl.a ]; then \
		echo "Configurando OpenSSL\n"; \
		cd include; \
		git clone $(OPENSSLREPO); \
		cd openssl; \
		echo $(PATH); \
		PATH=$(PATH) \
		AR=$(LLVMAR) \
		CC=$(CC) \
		NM=$(LLVMNM) \
		LDFLAGS="-L$(OPENSSL_INST)/" \
		CCFLAGS="-I$(OPENSSL_INST)/include" \
		RANLIB=$(LLVMRAMLIB) \
		ANDROID_NDK_ROOT=$(ANDROIDNDKROOT) \
		./Configure android-$(ARCHITECTURE) no-shared \
			-D__ANDROID_API__=$(ANDROIDAPILEVEL) \
			--prefix=$(OPENSSL_INST) \
			--openssldir=$(OPENSSL_INST) && \
		make build_libs -j$(nproc); \
		cd ../../; \
		echo "\n OpenSSL Configurado"; \
	fi;

	#Baixa e compila o curld;
	@if [ ! -e include/curl/lib/.libs/libcurl.a ] && [ -e include/openssl/libssl.a ]; then \
		echo "Configurando Curl\n"; \
		cd include; \
		git clone $(CURLREPO); \
		cd curl; \
		echo "Configurando Curl..."; \
		autoreconf -fi && \
		AR=$(LLVMAR) \
		CC=$(CC) \
		NM=$(LLVMNM) \
		LDFLAGS="-L$(OPENSSL_INST)/" \
		CCFLAGS="-I$(OPENSSL_INST)/include" \
		RANLIB=$(LLVMRAMLIB) \
		./configure --host=$(NDK_ARCH)-linux-android \
			--target=$(NDK_ARCH)-linux-android \
			--build=x86_64-pc-linux-gnu \
			--enable-static \
			--disable-shared \
			--without-libpsl \
			--with-openssl=$(OPENSSL_INST) \
			--prefix=$(WORKDIR)/include/curl/build_output && \
		make; \
		echo "\n Curl Configurado"; \
	else echo "Erro, falha ao compilar OpenSSL"; \
	fi;


# Phony targets don't correspond to actual files
.PHONY: all clean

# Clean target: removes generated files
clean:
	$(RM) $(TARGET) $(OBJS)