{
  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
  };

  outputs = { self, nixpkgs }:
  let 
    system  = "x86_64-linux";
    pkgs    = import nixpkgs { inherit system; };
  in
  {
    devShells.${system}.default = pkgs.mkShell { 
      nativeBuildInputs = with pkgs; [
        cmake
        clang-tools
        pkg-config
      ];
      buildInputs = with pkgs; [
        wayland
        libffi
        udev
        libevdev
        libxkbcommon
        luajit
      ];
      shellHook = ''
        alias m="cmake --build build && ./build/snippy"
        cmake -B build -S .
        echo "IMPORTANT: Make sure you are in the 'input' group"
        echo "Use m to build and run!"
      '';
    };
  };
}
