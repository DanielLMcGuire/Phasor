pkgname=phasor
pkgver=2.1.0
pkgrel=1
pkgdesc="Phasor Programming Language Toolchain"
arch=('x86_64')
url="https://github.com/DanielLMcGuire/Phasor"
license=('MIT')
depends=('cmake' 'ninja')
makedepends=('git' 'gcc')
source=("git+https://github.com/DanielLMcGuire/Phasor.git")
sha256sums=('SKIP')

pkgver() {
    cd "$srcdir/Phasor"
    tag=$(git describe --tags --abbrev=0 2>/dev/null || echo "0.0.0")
    commits_since_tag=$(git rev-list "${tag}"..HEAD --count 2>/dev/null || echo 0)
    if [ "$commits_since_tag" -eq 0 ]; then
        echo "$tag"
    else
        echo "${tag}.r${commits_since_tag}"
    fi
}


build() {
	cd "$srcdir/Phasor"
    cmake -S "$srcdir/Phasor" -B "$srcdir/Phasor/build" -G Ninja -DASSEMBLY=OFF
    cmake --build "$srcdir/Phasor/build"
}

package() {
    cd "$srcdir/Phasor/build"
    cmake --install . --prefix="$pkgdir/usr"

    for section in 1 3 5 7; do
        src="$srcdir/Phasor/docs/man/man$section"
        dest="$pkgdir/usr/local/share/man/man$section"
        mkdir -p "$dest"
        for file in "$src"/*."$section"; do
            [ -f "$file" ] && install -Dm644 "$file" "$dest"/
        done
    done
}


