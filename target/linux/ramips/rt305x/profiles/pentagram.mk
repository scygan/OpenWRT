#
# Copyright (C) 2010 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

define Profile/P6367
NAME:=Pentagram Cerberus P 6367
PACKAGES:=kmod-rt2800-pci
endef

define Profile/P6367/Description
	Profile for Pentagram Cerberus P 6367.
endef
$(eval $(call Profile,P6367))
