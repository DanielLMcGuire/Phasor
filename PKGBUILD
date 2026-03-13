# Maintainer: Daniel McGuire <danielmcguire2023@gmail.com>
pkgname=phasor-dev
PACKAGER="Daniel McGuire <danielmcguire2023@gmail.com>"
pkgver=3.0.0.dev
pkgrel=1
pkgdesc="Phasor Programming Language Toolchain"
arch=('x86_64')
url="https://github.com/DanielLMcGuire/Phasor"
license=('0BSD')
makedepends=('git' 'gcc' 'cmake' 'ninja')
optdepends=('gcc: For building Phasor Native wrappers.')
conflicts=('phasor' 'phasor-git')
options=(strip !debug)
install=scripts/phasor.install
source=()
sha256sums=()

pkgver() {
    cd "$startdir"
    tag=$(git describe --tags --abbrev=0 2>/dev/null || echo "3.0.0")
    commits_since_tag=$(git rev-list "${tag}"..HEAD --count 2>/dev/null || echo 0)
    if [ "$commits_since_tag" -eq 0 ]; then
        echo "$tag.dev"
    else
        echo "${tag}.r${commits_since_tag}.dev"
    fi
}

build() {
    cd "$startdir"
    cmake -S "$startdir" -B "$startdir/build"
    cmake --build "$startdir/build"
}

package() {
    cd "$startdir"
    cmake --install "$startdir/build" --prefix  "$pkgdir"
    
    install -Dm644 "$startdir/src/Extensions/unix/phasor.magic" \
        "$pkgdir/usr/share/file/magic/phasor"
}
