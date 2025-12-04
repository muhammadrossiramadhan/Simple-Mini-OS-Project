![Simple-Mini-OS-Project](./src/ascii.png)

## [Overview]

Simple-Mini-OS-Project adalah proyek untuk membuat *mini OS* dengan kernel sederhana yang mendukung driver simulasi dan system call minimal. Proyek ini dijalankan di `QEMU` emulator (32-bit; untuk mempermudah development) dan dapat digunakan untuk melakukan benchmark performa dibandingkan OS modern seperti Linux, Windows, atau Android.

Fitur utama:

- Driver simulasi untuk I/O virtual
- System call minimal:
  - `print` —> menampilkan teks ke layar
  - `read` —> membaca input
  - `allocate` —> alokasi memori sederhana
  - `create_task` —> membuat task baru
- Logging hasil benchmark:
  - Waktu akses file
  - Total execution time
- Menampilkan boot screen mini OS
- Task execution sederhana

## [Project Structure]

Simple-Mini-OS-Project \
├── boot\
│   └── grub\
│       └── grub.cfg\
├── build\
│   ├── boot.o\
│   ├── driver.o\
│   ├── kernel.bin\
│   ├── kernel.o\
│   ├── keyboard.o\
│   ├── scheduler.o\
│   └── syscall.o\
├── iso\
│   └── boot\
│       ├── grub\
│       │   └── grub.cfg\
│       └── kernel.bin\
├── kernel\
│   ├── boot.s\
│   ├── driver.c\
│   ├── driver.h\
│   ├── kernel.c\
│   ├── keyboard.c\
│   ├── keyboard.h\
│   ├── linker.ld\
│   ├── reboot.h\
│   ├── scheduler.c\
│   ├── scheduler.h\
│   ├── syscall.c\
│   └── syscall.h\
├── Makefile\
├── mini-os.iso\
├── minios_memory\
│   ├── main.c\
│   ├── memory_manager.c\
│   ├── memory_manager.h\
│   └── program.exe\
└── README.md

## [Direct Download]

- [Latest release (v1.0.0)](https://github.com/muhammadrossiramadhan/Simple-Mini-OS-Project/releases/tag/v1.0.0 "Open Release")

## [Manual Build]

0. **Dependencies**:
   - `WSL` (Windows Subsystem for Linux)
   - `GCC` & `NASM` — untuk compile kernel
   - `LD` (linker) — untuk linking binary
   - `QEMU` (32-bit) — untuk menjalankan mini-OS
   - `GRUB/GRUB2` — membuat bootable ISO
   - `xorriso` — membuat ISO bootable

1. **Clone repository** \
   Buka terminal lalu jalankan:
   ```
   git clone https://github.com/muhammadrossiramadhan/Simple-Mini-OS-Project.git
   ```

2. **Masuk ke directory repository**
   ```
   cd Simple-Mini-OS-Project
   ```

3. **Build kernel dan ISO** \
   Pastikan `Makefile` sudah tersedia, lalu jalankan:
   ```
   make clean; make && make run
   ```

---

### Rencana Pengembangan (Future Plan)

Beberapa pengembangan yang bisa dilakukan pada proyek Mini OS ini di masa depan:
- Menambah fitur sederhana seperti menu awal atau command kecil.
- Menambahkan komentar pada kode untuk mempermudah pembelajaran. (Agar lebih informatif)

### *Notes

*Mini-OS ini menggunakan GRUB sebagai bootloader.
Struktur proyek modular untuk memudahkan pengembangan driver atau system call tambahan.*
