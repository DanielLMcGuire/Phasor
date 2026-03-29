{
  description = "Phasor";
  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
    flake-utils.url = "github:numtide/flake-utils";
  };
  outputs = { self, nixpkgs, flake-utils }:
    flake-utils.lib.eachDefaultSystem (system:
      let
        pkgs = nixpkgs.legacyPackages.${system};
      in {
        packages.default = pkgs.stdenv.mkDerivation {
          pname = "phasor";
          version = "3.1.0";
          src = ./.;
          nativeBuildInputs = [
            pkgs.cmake
            pkgs.ninja
          ];
          cmakeFlags = [ "-DNIX=ON" ];
        };
      }
    );
}