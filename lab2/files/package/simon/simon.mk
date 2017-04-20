################################################################################
#
# simon
#
################################################################################

SIMON_VERSION = 1.4
SIMON_SITE = $(TOPDIR)/../simon
SIMON_SITE_METHOD = local

define SIMON_BUILD_CMDS
   $(MAKE) $(TARGET_CONFIGURE_OPTS) CFLAGS+="-std=gnu99" simon -C $(@D)
endef
define SIMON_INSTALL_TARGET_CMDS 
   $(INSTALL) -D -m 0755 $(@D)/simon $(TARGET_DIR)/usr/bin 
endef
SIMON_LICENSE = MIT

$(eval $(generic-package))
