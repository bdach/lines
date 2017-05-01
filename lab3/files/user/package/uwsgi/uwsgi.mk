################################################################################
#
# uwsgi
#
################################################################################

UWSGI_VERSION = 2.0.12
UWSGI_SOURCE = uwsgi-$(UWSGI_VERSION).tar.gz
UWSGI_SITE = https://pypi.python.org/packages/source/u/uWSGI
UWSGI_LICENSE = GPLv2
UWSGI_LICENSE_FILES = LICENSE

UWSGI_DEPENDENCIES = libxml2 host-python

UWSGI_ENV = \
	PATH=$(BR_PATH) \
	CC="$(TARGET_CC)" \
	CFLAGS="$(TARGET_CFLAGS)" \
	LDFLAGS="$(TARGET_LDFLAGS)" \
	LDSHARED="$(TARGET_CROSS)gcc -shared" \
	PYTHONPATH="$(if $(BR2_PACKAGE_PYTHON3),$(PYTHON3_PATH),$(PYTHON_PATH))" \
	_python_sysroot=$(STAGING_DIR) \
	_python_prefix=/usr \
	_python_exec_prefix=/usr \
	PCRE_CONFIG="$(STAGING_DIR)/usr/bin/pcre-config" \
	XML2_CONFIG="$(STAGING_DIR)/usr/bin/xml2-config"

define UWSGI_BUILD_CMDS
	$(MAKE) $(UWSGI_ENV) -C $(@D)
endef

define UWSGI_INSTALL_TARGET_CMDS
	$(INSTALL) -D -m 755 $(@D)/uwsgi  $(TARGET_DIR)/usr/bin/uwsgi
endef

$(eval $(generic-package))
