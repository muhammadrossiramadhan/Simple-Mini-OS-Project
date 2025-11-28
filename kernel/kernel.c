#include "syscall.h"
#include "keyboard.h"

void kernel_main() {
    clear_screen();
    sys_print("MiniOS aktif!\nKetik satu karakter: ");
    char c = sys_read();

    char msg[2] = {c, 0};
    sys_print("\nKamu menekan: ");
    sys_print(msg);


    while(1);
}
