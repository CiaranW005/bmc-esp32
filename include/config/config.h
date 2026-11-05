#pragma once

#include <Arduino.h>
#include <stdint.h>

#ifdef ORACLE_LOCAL
    #define ORACLE_HOST "192.168.1.67"
    #define ORACLE_PORT 8080
#else
    #define ORACLE_HOST "device.corasfein.com"
    #define ORACLE_PORT 443
#endif

// Trust anchor used to verify the production Oracle server certificate.
// The device's client certificate and private key are configured separately.
static const char BMC_ROOT_CA[] PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
MIIBpjCCAU2gAwIBAgIUeEMX11GSzqGFuB9o2359mBpD1NIwCgYIKoZIzj0EAwIw
ITEfMB0GA1UEAwwWQ29yYXMgRmVpbiBCTUMgUm9vdCBDQTAeFw0yNjA5MTcyMDUw
NDVaFw0zNjA5MTQyMDUwNDVaMCExHzAdBgNVBAMMFkNvcmFzIEZlaW4gQk1DIFJv
b3QgQ0EwWTATBgcqhkjOPQIBBggqhkjOPQMBBwNCAAT5qL/tttpiKKNTKfknwtrQ
voyrEusRmqVfqNQCAcxYhF3bUG5Lv5LN5LD7sMBU8q5fKizLPITrX8r9L5WgFeYm
o2MwYTAfBgNVHSMEGDAWgBTDb7Fjs+4kXCZzgwzVGju84GRY1zAPBgNVHRMBAf8E
BTADAQH/MA4GA1UdDwEB/wQEAwIBBjAdBgNVHQ4EFgQUw2+xY7PuJFwmc4MM1Ro7
vOBkWNcwCgYIKoZIzj0EAwIDRwAwRAIgIapHpzcuvHTdbINnLq5FQRu3B9R3eXSS
tkxZniNWlt8CIAMYAA47foZWEZ8R+W/OhA7cgqLjTvPPC5gRYCqnJn/N
-----END CERTIFICATE-----
)EOF";