{
  description = "RISC-V bare-metal kernel dev environment";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-24.05";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs = { self, nixpkgs, flake-utils }:
    flake-utils.lib.eachDefaultSystem (system:
      let
        pkgs = import nixpkgs { inherit system; };

        # Cross compiler toolchain (always available)
        riscv = pkgs.pkgsCross.riscv64-embedded.buildPackages;
      in
      {
        devShells.default = pkgs.mkShell {
          packages = [
            riscv.gcc
            riscv.binutils
            riscv.gdb

            pkgs.qemu
            pkgs.ccache
            pkgs.bear
          ];

          shellHook = ''
            echo "Loaded RISC-V toolchain (pkgsCross)"

            export CROSS_PREFIX="riscv64-unknown-elf-"

            export CC=riscv64-unknown-elf-gcc
            export LD=riscv64-unknown-elf-ld
            export AS=riscv64-unknown-elf-as
          '';
        };
      });
}

