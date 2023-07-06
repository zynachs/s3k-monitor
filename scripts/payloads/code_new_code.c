void mal_putchar(void) {
    /* Pointer to memory address of UART output, equivalent to putchar to stdout */
    char * ptr = (void *)0x10000000; 

    /* Translates to the string "Executing from out of bounds!" */
    *ptr = 0x45;
    *ptr = 0x78;
    *ptr = 0x65;
    *ptr = 0x63;
    *ptr = 0x75;
    *ptr = 0x74;
    *ptr = 0x69;
    *ptr = 0x6e;
    *ptr = 0x67;
    *ptr = 0x20;
    *ptr = 0x66;
    *ptr = 0x72;
    *ptr = 0x6f;
    *ptr = 0x6d;
    *ptr = 0x20;
    *ptr = 0x4e;
    *ptr = 0x45;
    *ptr = 0x57;
    *ptr = 0x20;
    *ptr = 0x63;
    *ptr = 0x6f;
    *ptr = 0x64;
    *ptr = 0x65;
    *ptr = 0x21;
    *ptr = 0x0a;

    return;
}