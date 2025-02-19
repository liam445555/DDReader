#
# Configuration file for using the XML library in GNOME applications
#
prefix="/usr"
exec_prefix="${prefix}"
libdir="${exec_prefix}/lib"
includedir="${prefix}/include"

XMLSEC_LIBDIR="${exec_prefix}/lib"
XMLSEC_INCLUDEDIR=" -D__XMLSEC_FUNCTION__=__FUNCTION__ -DXMLSEC_NO_GOST=1 -DXMLSEC_NO_CRYPTO_DYNAMIC_LOADING=1 -I${prefix}/include/xmlsec1   -I/usr/include/libxml2   -I/usr/include/libxml2   -I/usr/kerberos/include   -DXMLSEC_OPENSSL_098=1 -DXMLSEC_CRYPTO_OPENSSL=1 -DXMLSEC_CRYPTO=\\\"openssl\\\""
XMLSEC_LIBS="-L${exec_prefix}/lib -lxmlsec1-openssl -lxmlsec1   -lxml2   -lxslt -lz -lm -lxml2   -L/usr/kerberos/lib -lssl -lcrypto -ldl -lz  "
MODULE_VERSION="xmlsec-1.2.11-openssl"

