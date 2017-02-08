// EEPROM Addresses

#define ADDR_PETSCII       0
#define ADDR_HAYES_MENU    1
#define ADDR_BAUD_LO       2
#define ADDR_BAUD_HI       3
#define ADDR_PORT_LO       4
#define ADDR_PORT_HI       5
#define ADDR_MODEM_ECHO         10
#define ADDR_MODEM_FLOW         11
#define ADDR_MODEM_VERBOSE      12
#define ADDR_MODEM_QUIET        13
#define ADDR_MODEM_S0_AUTOANS   14
#define ADDR_MODEM_S2_ESCAPE    15
//#define ADDR_MODEM_DCD_INVERTED 16
#define ADDR_MODEM_DCD          17
#define ADDR_MODEM_X_RESULT     18
#define ADDR_MODEM_SUP_ERRORS   19
#define ADDR_MODEM_DSR          20

//#define ADDR_WIFI_SSID
//#define ADDR_WIFI_PASS

#define ADDR_HOST_AUTO          99     // Autostart host number
#define ADDR_HOSTS              100    // to 460 with ADDR_HOST_SIZE = 40 and ADDR_HOST_ENTRIES = 9
#define STATIC_PB_ENTRIES       2
#define ADDR_ANSWER_MESSAGE     800    // to 874 with ADDR_ANSWER_MESSAGE_SIZE = 75

#define ADDR_HOST_SIZE              40
#define ADDR_ANSWER_MESSAGE_SIZE    75              // Currently limited by the AT command line buffer size which is 80..
#define ADDR_HOST_ENTRIES           9
