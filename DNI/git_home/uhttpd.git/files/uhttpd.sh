#!/bin/sh

REALM=`/bin/cat /module_name | sed 's/\n//g'`
UHTTPD_BIN="/usr/sbin/uhttpd"
PX5G_BIN="/usr/sbin/px5g"

OPENSSL="/usr/bin/openssl"
KEY_FILE="/etc/uhttpd.key"
CRT_FILE="/etc/uhttpd.crt"

generate_keys() {
	[ -f $KEY_FILE -a -f $CRT_FILE ] || {
		#generate a 2048 bit rsa private/public key without pass-phrase encrypt to used for tls
		${OPENSSL} genrsa -out ${KEY_FILE} 2048
		#generate csr
		${OPENSSL} req -new -x509 -key ${KEY_FILE} -out ${CRT_FILE} -days 3650 -subj "/C=US/ST=California/L=San Jose/O=NETGEAR/OU=Home Consumer Products/CN=www.routerlogin.net/emailAddress=support@netgear.com"
	}
}

uhttpd_stop()
{
	kill -9 $(pidof uhttpd)
}

uhttpd_start()
{
	generate_keys
	$UHTTPD_BIN -h /www -r ${REALM}  -x /cgi-bin -t 80 -p 0.0.0.0:80 -C ${CRT_FILE} -K ${KEY_FILE} -s 0.0.0.0:443
}

case "$1" in
	stop)
		uhttpd_stop
	;;
	start)
		uhttpd_start
	;;
	restart)
		uhttpd_stop
		uhttpd_start
	;;
	*)
		logger -- "usage: $0 start|stop|restart"
	;;
esac

