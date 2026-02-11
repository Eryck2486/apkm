#Makefile para a criação do helper em JNI para o apkm
D8_CLASS=com.android.tools.r8.D8
RM=rm -rf
ifeq ($(ANDROIDLIB),)
	ANDROIDLIB=libs/android.jar
endif

all: clean prepare Helper.jar
	$(RM) src/*.class

#Transformando todos os arquivos .java em .class
*.class: src/*.java
	$(JAVAC) -cp "$(ANDROIDLIB)" src/*.java

#Criando classe java para ART VM
Helper.jar: *.class

	$(JAVA) -cp $(D8) $(D8_CLASS) src/*.class --output build/Helper.jar

clean:
	$(RM) nuild/
	$(RM) src/*.class

prepare:
ifeq ($(JAVA),)
#Definindo java padrão do sistema
	$(error binário java não especificado em JAVA=)
endif
ifeq ($(JAVAC),)
#Definindo javac padrão do sistema
	$(error binário javac não especificado em JAVAC=)
endif
ifeq ($(D8),)
	$(error d8.jar não especificado em D8=)
endif
#Verificando se androidlib foi especificado, caso contrario define como libs/android.jar
	@mkdir -p build