include $(TOPDIR)/rules.mk

PKG_NAME:=openwrt-package-template
PKG_TITLE:=Openwrt package template
PKG_RELEASE:=2.0

PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)
PKG_CONFIG_DEPENDS := 

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk

define Package/$(PKG_NAME)
  SECTION:=net
  CATEGORY:=Network
  TITLE:=$(PKG_TITLE)
  DEPENDS:=+libuci
  URL:= httt://bjbook.net/openwrt
  MAINTAINER:=zhang <unflypig12138@gmail.com>
endef

define Package/$(PKG_NAME)/description
 This is OpenWrt package template.
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
	$(CP) ./lib/ $(PKG_BUILD_DIR)/
	$(CP) ./inc/ $(PKG_BUILD_DIR)/
	$(CP) ./files/ $(PKG_BUILD_DIR)/
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_CONF) ./files/$(PKG_NAME).conf $(1)/etc/config/$(PKG_NAME)
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/$(PKG_NAME).init $(1)/etc/init.d/$(PKG_NAME)
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/$(PKG_NAME) $(1)/usr/sbin/$(PKG_NAME)
	$(INSTALL_DIR) $(1)/lib
	$(INSTALL_BIN) ./lib/libmqttclient.so $(1)/lib/libmqttclient.so
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
