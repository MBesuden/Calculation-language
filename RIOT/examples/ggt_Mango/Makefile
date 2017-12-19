#Eclipse settings see: https://github.com/RIOT-OS/RIOT/wiki/Using-the-Eclipse-IDE-for-C-and-CPP-Developers,-Howto
# name of your application
APPLICATION = ggt_Mango

# If no BOARD is found in the environment, use this default:
BOARD ?= native

# This has to be the absolute path to the RIOT base directory:
RIOTBASE ?= $(CURDIR)/../..

USEMODULE += xtimer

#Shell
USEMODULE += shell
USEMODULE += shell_commands
USEMODULE += ps

# Comment this out to disable code in RIOT that does safety checking
# which is not needed in a production environment but helps in the
# development process:
CFLAGS += -DDEVELHELP
CFLAGS += -Wno-pedantic -Wno-error=pedantic
# Change this to 0 show compiler invocation lines by default:
QUIET ?= 1


include $(RIOTBASE)/Makefile.include
