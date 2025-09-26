TARGET = kernel.bin
ISO = myos.iso

# Détection de l'OS et de l'environnement
ifeq ($(OS),Windows_NT)
    DETECTED_OS = Windows
    RM = del /Q
    RMDIR = rmdir /S /Q
    MKDIR = mkdir
    PATH_SEP = \\
else
    DETECTED_OS = Unix
    RM = rm -f
    RMDIR = rm -rf
    MKDIR = mkdir -p
    PATH_SEP = /
endif

# Détection du cross-compiler et configuration automatique
CC_TEST := $(shell which x86_64-elf-gcc 2>/dev/null)
ifeq ($(CC_TEST),)
    # Cross-compiler pas trouvé, essayer des alternatives
    GCC_TEST := $(shell which gcc 2>/dev/null)
    ifneq ($(GCC_TEST),)
        CC = gcc
        LD = ld
        CFLAGS_ARCH = -m32
        LD_ARCH = -m elf_i386
        TOOLCHAIN_AVAILABLE = native
    else
        CC = x86_64-elf-gcc
        LD = x86_64-elf-ld
        CFLAGS_ARCH = -m32
        LD_ARCH = -m elf_i386
        TOOLCHAIN_AVAILABLE = missing
    endif
else
    CC = x86_64-elf-gcc
    LD = x86_64-elf-ld
    CFLAGS_ARCH = -m32
    LD_ARCH = -m elf_i386
    TOOLCHAIN_AVAILABLE = cross
endif

AS = nasm

# Flags pour les compilateurs (optimisation maximale)
CFLAGS = $(CFLAGS_ARCH) -ffreestanding -O3 -Wall -Wextra -c -I. -funsigned-char -fno-stack-protector -fomit-frame-pointer -funroll-loops
ASFLAGS = -f elf32

# Liste de tous les fichiers sources
C_SOURCES = kernel.c idt.c gdt.c io.c timer.c keyboard.c game.c pong.c health.c blackjack.c memory.c editor.c filemanager.c password.c messaging.c wiki.c
S_SOURCES = boot.s gdt_asm.s interrupts.s
OBJECTS = $(C_SOURCES:.c=.o) $(S_SOURCES:.s=.o)

# Règle par défaut
all: check-toolchain $(ISO)

# Vérification et installation automatique des outils
check-toolchain:
ifeq ($(TOOLCHAIN_AVAILABLE),missing)
	@echo "Cross-compiler manquant ! Installation automatique..."
	@make install-toolchain
endif
	@echo "Toolchain status: $(TOOLCHAIN_AVAILABLE)"

# Installation automatique du cross-compiler
install-toolchain:
	@echo "Installation du cross-compiler..."
	apt-get update >/dev/null 2>&1 || true
	apt-get install -y build-essential nasm gcc-multilib binutils >/dev/null 2>&1 || true
	# Essayer d'installer le cross-compiler x86_64-elf si disponible
	apt-get install -y gcc-x86-64-linux-gnu >/dev/null 2>&1 || true
	@echo "Installation terminée ! Relancez make."

# Alternative: construire avec gcc natif si le cross-compiler n'est pas disponible
build-native: 
	@echo "Construction avec gcc natif (fallback)..."
	$(MAKE) CC=gcc LD=ld CFLAGS_ARCH="-m32" LD_ARCH="-m elf_i386" TOOLCHAIN_AVAILABLE=native $(ISO)

# Règles de compilation génériques
%.o: %.c
	$(CC) $(CFLAGS) $< -o $@

%.o: %.s
	$(AS) $(ASFLAGS) $< -o $@

# Règle pour l'édition des liens (linking)
$(TARGET): $(OBJECTS) linker.ld
	$(LD) $(LD_ARCH) -T linker.ld -o $(TARGET) $(OBJECTS)

# Règle COMPLÈTE pour créer l'image ISO
$(ISO): $(TARGET)
ifeq ($(DETECTED_OS),Windows)
	if exist isofiles $(RMDIR) isofiles 2>nul
	$(MKDIR) isofiles$(PATH_SEP)boot$(PATH_SEP)grub
	copy $(TARGET) isofiles$(PATH_SEP)boot$(PATH_SEP)kernel.bin
else
	$(RMDIR) isofiles
	$(MKDIR) isofiles/boot/grub
	cp $(TARGET) isofiles/boot/kernel.bin
endif
	echo 'set timeout=0' > isofiles/boot/grub/grub.cfg
	echo 'set default=0' >> isofiles/boot/grub/grub.cfg
	echo 'menuentry "NOVA" {' >> isofiles/boot/grub/grub.cfg
	echo '  multiboot /boot/kernel.bin' >> isofiles/boot/grub/grub.cfg
	echo '  boot' >> isofiles/boot/grub/grub.cfg
	echo '}' >> isofiles/boot/grub/grub.cfg
	@make create-iso

# Création de l'ISO avec détection automatique des outils
create-iso:
	@echo "Création de l'image ISO..."
	@if command -v grub-mkrescue >/dev/null 2>&1; then \
		echo "Utilisation de grub-mkrescue local..."; \
		grub-mkrescue -o $(ISO) isofiles; \
	elif command -v docker >/dev/null 2>&1; then \
		echo "Utilisation de Docker pour grub-mkrescue..."; \
		docker run --rm --platform=linux/amd64 \
			-v "$(PWD)":/work -w /work debian:stable-slim \
			bash -lc "apt-get update >/dev/null && apt-get install -y grub-pc-bin xorriso >/dev/null && \
					  grub-mkrescue -o $(ISO) isofiles"; \
	else \
		echo "Installation de grub-mkrescue..."; \
		apt-get update >/dev/null 2>&1 || true; \
		apt-get install -y grub-pc-bin xorriso >/dev/null 2>&1 || true; \
		if command -v grub-mkrescue >/dev/null 2>&1; then \
			grub-mkrescue -o $(ISO) isofiles; \
		else \
			echo "ERREUR: Impossible de créer l'ISO. Installez manuellement grub-pc-bin et xorriso."; \
			exit 1; \
		fi; \
	fi

# Installation manuelle des outils GRUB
install-grub:
	@echo "Installation des outils GRUB..."
	apt-get update >/dev/null 2>&1 || true
	apt-get install -y grub-pc-bin xorriso >/dev/null 2>&1 || true
	@echo "Installation terminée !"

# Règle pour lancer QEMU normalement (optimisé)
run: $(ISO)
ifeq ($(DETECTED_OS),Windows)
	qemu-system-x86_64 -cdrom $(ISO) -m 32768M -smp 64 -cpu max -machine pc,accel=whpx 2>nul || qemu-system-x86_64 -cdrom $(ISO) -m 32768M -smp 64 -cpu max -machine pc
else
	qemu-system-x86_64 -cdrom $(ISO) -m 32768M -smp 64 -cpu max -machine pc,accel=hvf 2>/dev/null || qemu-system-x86_64 -cdrom $(ISO) -m 32768M -smp 64 -cpu max -machine pc
endif

# Règle pour lancer QEMU avec MAXIMUM de ressources
run-power: $(ISO)
ifeq ($(DETECTED_OS),Windows)
	qemu-system-x86_64 -cdrom $(ISO) -m 65536M -smp 128 -cpu max -machine pc,accel=whpx 2>nul || qemu-system-x86_64 -cdrom $(ISO) -m 65536M -smp 128 -cpu max -machine pc
else
	qemu-system-x86_64 -cdrom $(ISO) -m 65536M -smp 128 -cpu max -machine pc,accel=hvf 2>/dev/null || qemu-system-x86_64 -cdrom $(ISO) -m 65536M -smp 128 -cpu max -machine pc
endif

# Règle pour macOS avec tentative d'accélération HVF
run-mac: $(ISO)
	qemu-system-x86_64 -cdrom $(ISO) -m 65536M -smp 128 -cpu max -machine pc,accel=hvf 2>/dev/null || qemu-system-x86_64 -cdrom $(ISO) -m 65536M -smp 128 -cpu max -machine pc

# Règle pour Windows avec accélération WHPX
run-windows: $(ISO)
	qemu-system-x86_64 -cdrom $(ISO) -m 65536M -smp 128 -cpu max -machine pc,accel=whpx 2>nul || qemu-system-x86_64 -cdrom $(ISO) -m 65536M -smp 128 -cpu max -machine pc

# Règle pour lancer QEMU en mode débogage
debug: $(TARGET)
	qemu-system-x86_64 -kernel $(TARGET) -S -s -m 4096M -smp 8 -cpu max

# Règle pour nettoyer le répertoire
clean:
ifeq ($(DETECTED_OS),Windows)
	if exist *.o $(RM) *.o 2>nul
	if exist *.bin $(RM) *.bin 2>nul
	if exist *.iso $(RM) *.iso 2>nul
	if exist isofiles $(RMDIR) isofiles 2>nul
else
	$(RM) *.o *.bin *.iso
	$(RMDIR) isofiles
endif

# Information sur l'OS et toolchain détectés
info:
	@echo "=== INFORMATIONS SYSTÈME ==="
	@echo "OS détecté: $(DETECTED_OS)"
	@echo "Toolchain: $(TOOLCHAIN_AVAILABLE)"
	@echo "Compilateur C: $(CC)"
	@echo "Éditeur de liens: $(LD)"
	@echo "Assembleur: $(AS)"
ifeq ($(DETECTED_OS),Windows)
	@echo "Commandes système: del, rmdir, mkdir, copy"
	@echo "Accélération QEMU: WHPX (Windows Hypervisor Platform)"
else
	@echo "Commandes système: rm, mkdir, cp"
	@echo "Accélération QEMU: HVF (Hypervisor Framework)"
endif

# Aide avec toutes les cibles disponibles
help:
	@echo "=== CIBLES DISPONIBLES ==="
	@echo "make            - Compiler l'OS (avec détection automatique)"
	@echo "make info       - Afficher les informations système"
	@echo "make install-toolchain - Installer les outils de compilation"
	@echo "make build-native      - Forcer compilation avec gcc natif"
	@echo "make run        - Lancer l'OS dans QEMU"
	@echo "make run-power  - Lancer avec plus de ressources"
	@echo "make run-windows - Lancer optimisé pour Windows"
	@echo "make run-mac    - Lancer optimisé pour macOS"
	@echo "make debug      - Lancer en mode débogage"
	@echo "make clean      - Nettoyer les fichiers générés"
