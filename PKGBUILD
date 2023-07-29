pkgname=vip
pkgver=0.0.1
pkgrel=1
arch=("x86_64")

pkgdesc="A virtual input panel for KDE Wayland based on QT5"
url="https://github.com/9r0x/vip"
license=("")

depends=("qt5-base")
makedepends=("extra-cmake-modules" "cmake")

source=("https://github.com/9r0x/vip/archive/refs/tags/v0.0.1.tar.gz")
sha256sums=("6333f2fe71dfb983f697850d98325eb8b5237ae637e04a981cd48ffad8e2511f")

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
  sudo mkdir -p /etc/vip.d
  sudo cp -r "$srcdir/config" /etc/vip.d
  sudo chown root:root "$pkgdir/usr/bin/vip"
  sudo chmod 4755 /usr/bin/vip
}
