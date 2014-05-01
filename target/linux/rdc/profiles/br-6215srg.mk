#
# Copyright (C) 2007 DAC System Tomasz Kosiak Company 
#	http://dacsystem.pl, 		Poland
# Author: Slawomir Cygan 	<slaffek[at]gmail.com>
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

define Profile/br6215srg
  NAME:=Edimax BR-6215SRG
  PACKAGES:=kmod-rt61-pci kmod-r6040 kmod-usb-core kmod-usb-ohci kmod-usb2
endef

define Profile/br6215srg/description
  Packages set compatible with the Edimax BR-6215SRG router
endef

$(eval $(call Profile,br6215srg))
