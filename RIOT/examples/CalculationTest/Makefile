#Eclipse settings see: https://github.com/RIOT-OS/RIOT/wiki/Using-the-Eclipse-IDE-for-C-and-CPP-Developers,-Howto
# name of your application
APPLICATION = CalculationTest

# If no BOARD is found in the environment, use this default:
BOARD ?= native

# This has to be the absolute path to the RIOT base directory:
RIOTBASE ?= $(CURDIR)/../..

# use ethos (ethernet over serial) for network communication and stdio over
# UART, but not on native, as native has a tap interface towards the host.
ifeq (,$(filter native,$(BOARD)))
GNRC_NETIF_NUMOF := 1
USEMODULE += ethos gnrc_netdev2

# ethos baudrate can be configured from make command
ETHOS_BAUDRATE ?= 115200
CFLAGS += '-DETHOS_UART=UART_DEV(0)' -DETHOS_BAUDRATE=$(ETHOS_BAUDRATE) -DUSE_ETHOS_FOR_STDIO
FEATURES_REQUIRED += periph_uart
endif

#Used Packages: use make BOARD=<> eclipsesym, and import settings after change of modules or board
# Include packages that pull up and auto-init the link layer.
# NOTE: 6LoWPAN will be included if IEEE802.15.4 devices are present
USEMODULE += gnrc_netdev_default
USEMODULE += auto_init_gnrc_netif
# Specify the mandatory networking modules
USEMODULE += gnrc_ipv6_default
USEMODULE += gnrc_icmpv6_echo

USEMODULE += xtimer

#gcoap
USEMODULE += gcoap
USEMODULE += od
USEMODULE += fmt

#Shell
USEMODULE += shell
USEMODULE += shell_commands
USEMODULE += ps

#nanocoap used by gcoap
USEPKG += nanocoap
# Required by nanocoap, but only due to issue #5959.
USEMODULE += posix
# Required by nanocoap to compile nanocoap_sock.
USEMODULE += gnrc_sock_udp


#include header files
#CFLAGS += -I$(CURDIR)/includes

# Comment this out to disable code in RIOT that does safety checking
# which is not needed in a production environment but helps in the
# development process:
CFLAGS += -DDEVELHELP

# Change this to 0 show compiler invocation lines by default:
QUIET ?= 1


include $(RIOTBASE)/Makefile.include
