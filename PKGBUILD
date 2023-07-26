pkgname=vip
pkgver=0.0.1
pkgrel=1
arch=("x86_64")

pkgdesc="A virtual input panel for KDE Wayland based on QT5"
url="https://github.com/9r0x/vip"
license=("")

depends=("qt5-base")
makedepends=("extra-cmake-modules" "cmake")

#source=("")
#sha256sums=("")

build() {
  cd "$srcdir/vip-$pkgver"
  mkdir build
  cd build
  cmake -DCMAKE_INSTALL_PREFIX=/usr ..
  make
}

package() {
  cd "$srcdir/vip-$pkgver/build"
  make DESTDIR="$pkgdir/" install
}

post_install() {
  sudo chown root:root "$pkgdir/usr/bin/vip"
  sudo chmod 4755 /usr/bin/vip
}