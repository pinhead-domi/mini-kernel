{
  description = "RISC-V kernel development environment";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs = { self, nixpkgs, flake-utils }:
    flake-utils.lib.eachDefaultSystem (system:
      let
        pkgs = import nixpkgs {
          inherit system;
        };
        
        # Build the correct RISC-V toolchain
        riscv-toolchain = pkgs.pkgsCross.riscv64-embedded.buildPackages.gcc;
        
      in {
        devShells.default = pkgs.mkShell {
          buildInputs = with pkgs; [
            riscv-toolchain
            qemu
            gdb
            gnumake
            bear
          ];
          
          shellHook = ''
            # Create symlink with the expected name
            mkdir -p $PWD/.devshell/bin
            ln -sf ${riscv-toolchain}/bin/riscv64-unknown-elf-gcc $PWD/.devshell/bin/riscv64-unknown-elf-gcc 2>/dev/null || true
            export PATH=$PWD/.devshell/bin:$PATH
            
            echo "RISC-V kernel development environment loaded"
            riscv64-unknown-elf-gcc --version
          '';
        };
      }
    );
}
