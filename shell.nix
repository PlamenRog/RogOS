{ pkgs ? import <nixpkgs> {} }:

pkgs.mkShell {
  nativeBuildInputs = with pkgs; [
    pkgsCross.i686-embedded.buildPackages.gcc
    pkgsCross.i686-embedded.buildPackages.binutils

    gnumake
    python3

    grub2
    xorriso
    mtools

    qemu
  ];
}
