include $(TOPDIR)/rules.mk

PKG_NAME:=hello
PKG_RELEASE:=2.0

PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)
PKG_CONFIG_DEPENDS := 

include $(INCLUDE_DIR)/package.mk

define Package/hello
  SECTION:=net
  CATEGORY:=Network
  TITLE:=Hello OpenWrt demo utility
  DEPENDS:=+libuci
  URL:= httt://bjbook.net/openwrt
  MAINTAINER:=zhang <unflypig12138@gmail.com>
endef

define Package/hello/description
 This is Route Hello World OpenWrt.
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

define Package/hello/install
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_CONF) ./files/hello.conf $(1)/etc/config/hello
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/hello.init $(1)/etc/init.d/hello
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/hello $(1)/usr/sbin/hello
endef

$(eval $(call BuildPackage,hello))
