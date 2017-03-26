################################################################################
#
# myrubyscript
#
################################################################################

MYRUBYSCRIPT_VERSION = 1.0
MYRUBYSCRIPT_SITE = $(TOPDIR)/../myrubyscript
MYRUBYSCRIPT_SITE_METHOD = local
MYRUBYSCRIPT_DEPENDENCIES = ruby

define MYRUBYSCRIPT_INSTALL_TARGET_CMDS
	$(INSTALL) -D -m 0755 $(@D)/hello.rb $(TARGET_DIR)/usr/bin
endef
MYRUBYSCRIPT_LICENSE = MIT

$(eval $(generic-package))
