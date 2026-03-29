{
  description = "Phasor";
  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
  };
  outputs = { self, nixpkgs }:
    let
      system = "x86_64-linux";
      pkgs = nixpkgs.legacyPackages.${system};
    in {
      packages.${system}.default = pkgs.stdenv.mkDerivation {
        pname = "phasor";
        version = "3.1.0";
        src = ./.;
        nativeBuildInputs = [
          pkgs.cmake
          pkgs.ninja
        ];
        preConfigure = ''
          export AR="gcc-ar"
          export RANLIB="gcc-ranlib"
          export NM="gcc-nm"
        '';
      };
    };
}
