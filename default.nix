let
  pkgs = import <nixpkgs> { };
in
{
    qtmips = pkgs.callPackage (import extras/packaging/nix/qtmips.nix) { };
}
