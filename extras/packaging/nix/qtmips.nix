{ libsForQt5, cmake, qt5, lib }:
with libsForQt5;
mkDerivation rec {
    name = "QtMips-Di";
    src = builtins.fetchGit ../../..;
    nativeBuildInputs = [ cmake ];
    buildInputs = [ qt5.qtbase qt5.qtsvg ];
    meta = {
        description = "MIPS CPU simulator for education purposes.";
        longDescription = ''
          MIPS CPU simulator for education purposes with pipeline and cache visualization.
          Developed at FEE CTU for computer architecturs classes.
        '';
        homepage = "https://github.com/kchasialis/QtMips-Di";
        license = lib.licenses.gpl3Plus;
        maintainers = [ "Kostas Chasialis <koschasialis@gmail.com>" ];
    };
}