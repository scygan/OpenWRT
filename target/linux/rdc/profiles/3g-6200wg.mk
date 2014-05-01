#
# Copyright (C) 2007 DAC System Tomasz Kosiak Company 
#	http://dacsystem.pl, 		Poland
# Author: Slawomir Cygan 	<slaffek[at]gmail.com>
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

define Profile/3g6200wg
  NAME:=Edimax 3G-6200WG
  PACKAGES:=kmod-rt61-pci kmod-r6040 kmod-usb-core kmod-usb-ohci kmod-usb2
endef

define Profile/3g6200wg/description
  Packages set compatible with the Edimax 3G-6200WG router
endef

$(eval $(call Profile,3g6200wg))
