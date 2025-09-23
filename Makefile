TARGET = kernel.bin
ISO = myos.iso
CC = x86_64-elf-gcc
AS = nasm

# Flags pour les compilateurs (avec débogage activé)
CFLAGS = -m32 -ffreestanding -O2 -Wall -Wextra -c -I. -funsigned-char -g
ASFLAGS = -f elf32

# Liste de tous les fichiers sources
C_SOURCES = kernel.c idt.c gdt.c io.c timer.c keyboard.c game.c
S_SOURCES = boot.s gdt_asm.s interrupts.s
OBJECTS = $(C_SOURCES:.c=.o) $(S_SOURCES:.s=.o)

# Règle par défaut
all: $(ISO)

# Règles de compilation génériques
%.o: %.c
	$(CC) $(CFLAGS) $< -o $@

%.o: %.s
	$(AS) $(ASFLAGS) $< -o $@

# Règle pour l'édition des liens (linking)
$(TARGET): $(OBJECTS) linker.ld
	x86_64-elf-ld -m elf_i386 -T linker.ld -o $(TARGET) $(OBJECTS)

# Règle COMPLÈTE pour créer l'image ISO
$(ISO): $(TARGET)
	rm -rf isofiles
	mkdir -p isofiles/boot/grub
	cp $(TARGET) isofiles/boot/kernel.bin
	echo 'set timeout=0' > isofiles/boot/grub/grub.cfg
	echo 'set default=0' >> isofiles/boot/grub/grub.cfg
	echo 'menuentry "MyOS" {' >> isofiles/boot/grub/grub.cfg
	echo '  multiboot /boot/kernel.bin' >> isofiles/boot/grub/grub.cfg
	echo '  boot' >> isofiles/boot/grub/grub.cfg
	echo '}' >> isofiles/boot/grub/grub.cfg
	docker run --rm --platform=linux/amd64 \
		-v "$(PWD)":/work -w /work debian:stable-slim \
		bash -lc "apt-get update >/dev/null && apt-get install -y grub-pc-bin xorriso >/dev/null && \
				  grub-mkrescue -o $(ISO) isofiles"

# Règle pour lancer QEMU normalement
run: $(ISO)
	qemu-system-x86_64 -cdrom $(ISO)

# Règle pour lancer QEMU en mode débogage
debug: $(TARGET)
	qemu-system-x86_64 -kernel $(TARGET) -S -s

# Règle pour nettoyer le répertoire
clean:
	rm -f *.o *.bin *.iso
	rm -rf isofiles