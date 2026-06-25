# ----------------------------
# Makefile Options
# ----------------------------

NAME = FIFTEEN
ICON = icon.png
DESCRIPTION = "Fifteen game"
COMPRESSED = YES
COMPRESSED_MODE = zx7
ARCHIVED = YES

CFLAGS = -Wall -Wextra -Oz
CXXFLAGS = -Wall -Wextra -Oz

# ----------------------------

include $(shell cedev-config --makefile)
