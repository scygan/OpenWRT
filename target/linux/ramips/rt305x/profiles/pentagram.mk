#
# Copyright (C) 2014 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

define Profile/P6367
	NAME:=Pentagram Cerberus P6467
	PACKAGES:=\
		kmod-usb-core kmod-usb-rt305x-dwc_otg \
		kmod-ledtrig-usbdev
endef

define Profile/P6367/Description
	Package set for Pentagram Cerberus P 6367 Broadband Router
endef

$(eval $(call Profile,P6367))
