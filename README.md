# RogOS
RogOS is a small non-graphical x86 protected-mode operating system.

## Dependencies & runtime requirements
 - GCC i686-elf cross-compiler with binutils
 - GNU Make
 - Python 3
 - Grub2
 - GNU xorriso
 - GNU mtools
 - qemu

## Building
It is recommended to use the nix shell file for building the project, as compiling with the regular gcc you may find in your distro's repostories more than likely won't work, and so you need to build the cross-compiler. If you do end up going that route, refer to: https://wiki.osdev.org/GCC_Cross-Compiler.

Clang is optionally used for code formatting([clang-format](https://clang.llvm.org/docs/ClangFormat.html) binary). You could also theoretically use it for compilation, but some code chunks will need to be rewritten as GCC functionality is used for stack smashing protection.

The Grub2 bootloader is used for booting, but it shouldn't be too difficult to swap out.

Then you may compile and boot into the OS with:
```sh
nix-shell --run './qemu.sh'
```

## References
 - https://wiki.osdev.org/
 - https://gitlab.com/sortie/meaty-skeleton
 - https://www.brokenthorn.com/Resources/OSDevIndex.html

