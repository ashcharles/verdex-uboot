#
# Copyright (C) Freescale Semiconductor, Inc. 2006. All rights reserved.
#
# See file CREDITS for list of people who contributed to this
# project.
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; either version 2 of
# the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston,
# MA 02111-1307 USA
#

#
# MPC8349ITX
#

TEXT_BASE  =   0xFEF00000

ifneq ($(OBJTREE),$(SRCTREE))
# We are building u-boot in a separate directory, use generated
# .lds script from OBJTREE directory.
LDSCRIPT := $(OBJTREE)/board/$(BOARDDIR)/u-boot.lds
endif
