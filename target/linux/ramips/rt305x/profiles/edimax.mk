#
# Copyright (C) 2010 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

define Profile/3G6200N
NAME:=Edimax 3G-6200n
PACKAGES:=kmod-rt2800-pci
endef

define Profile/3G6200N/Description
	Profile for Edimax 3G-6200n.
endef
$(eval $(call Profile,3G6200N))
