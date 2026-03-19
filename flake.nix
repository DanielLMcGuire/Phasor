packages.default = pkgs.stdenv.mkDerivation {
  pname = "phasor";
  version = "3.1.0";

  src = ./.;

  nativeBuildInputs = [
    pkgs.cmake
    pkgs.ninja
  ];
};