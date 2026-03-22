# Maintainer: Daniel McGuire <danielmcguire2023@gmail.com>
pkgname=phasor-dev
PACKAGER="Daniel McGuire <danielmcguire2023@gmail.com>"
pkgver=3.1.0.dev
pkgrel=1
pkgdesc="Phasor Programming Language Toolchain"
arch=('x86_64')
url="https://github.com/DanielLMcGuire/Phasor"
license=('0BSD')
makedepends=('git' 'gcc' 'cmake' 'ninja' 'python' 'python-installer' 'python-build')
optdepends=('gcc: For building Phasor Native wrappers.' 'python: For manipulating bytecode' 'lief: For extracting native bytecode')
conflicts=('phasor' 'phasor-git')
options=(strip !debug)
install=scripts/phasor.install
source=()
sha256sums=()

pkgver() {
    cd "$startdir"
    tag=$(git describe --tags --abbrev=0 2>/dev/null || echo "3.1.0")
    commits_since_tag=$(git rev-list "${tag}"..HEAD --count 2>/dev/null || echo 0)
    if [ "$commits_since_tag" -eq 0 ]; then
        echo "$tag.dev"
    else
        echo "${tag}.r${commits_since_tag}.dev"
    fi
}

prepare() {
    cd "$startdir"
    git submodule update --init --recursive
}

build() {
    cd "$startdir"
    "/usr/bin/python" "$startdir/pmake-bootstrap.py" --native
    chmod +x "$startdir/pmake"
    "$startdir/pmake" linux-64-rel -s "$startdir" -b

    cd "$startdir/src/Extensions/py/phasor"
    "/usr/bin/python" -m build --wheel
}

package() {
    cd "$startdir"
    "$startdir/pmake" -i "$pkgdir" 
    
    install -Dm644 "$startdir/src/Extensions/unix/phasor.magic" \
        "$pkgdir/usr/share/file/misc/magic/phasor"

    python -m installer --destdir="$pkgdir" "$startdir/src/Extensions/py/phasor/dist/"*.whl
}
