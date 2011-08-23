#
# This is config used for compilation of Gumstix sources
#
# You might change location of U-Boot in memory by setting right TEXT_BASE.
# This allows for example having one copy located at the end of ram and stored
# in flash device and later on while developing use other location to test
# the code in RAM device only.
#
BR2_TARGET_GUMSTIX_VERDEX=y
GUMSTIX_600MHZ=y

ifeq ($(BR2_TARGET_GUMSTIX_VERDEX),y)
	TEXT_BASE  = 0x5C000000
	PLATFORM_CPPFLAGS += -DGUMSTIX_VERDEX
endif
ifeq ($(BR2_TARGET_GUMSTIX_BASIXCONNEX),y)
	TEXT_BASE  = 0xA3F00000
	PLATFORM_CPPFLAGS += -DGUMSTIX_PXA255
endif
ifeq ("$(BR2_TARGET_GUMSTIX_VERDEX)$(BR2_TARGET_GUMSTIX_BASIXCONNEX)","")
     $(error You need to specify either BR2_TARGET_GUMSTIX_BASIXCONNEX or BR2_TARGET_GUMSTIX_VERDEX)
endif
ifeq ($(GUMSTIX_200MHZ),y)
	PLATFORM_CPPFLAGS += -DCONFIG_GUMSTIX_CPUSPEED_200
endif
ifeq ($(GUMSTIX_300MHZ),y)
	PLATFORM_CPPFLAGS += -DCONFIG_GUMSTIX_CPUSPEED_300
endif
ifeq ($(GUMSTIX_400MHZ),y)
	PLATFORM_CPPFLAGS += -DCONFIG_GUMSTIX_CPUSPEED_400
endif
ifeq ($(GUMSTIX_500MHZ),y)
	PLATFORM_CPPFLAGS += -DCONFIG_GUMSTIX_CPUSPEED_500
endif
ifeq ($(GUMSTIX_600MHZ),y)
	PLATFORM_CPPFLAGS += -DCONFIG_GUMSTIX_CPUSPEED_600
endif
ifeq ("$(GUMSTIX_200MHZ)$(GUMSTIX_300MHZ)$(GUMSTIX_400MHZ)$(GUMSTIX_500MHZ)$(GUMSTIX_600MHZ)","")
     $(error You need to specify a CPU speed via GUMSTIX_nnnMHZ)
endif
PLATFORM_CPPFLAGS += -DSVN_REVISION='"$(SVN_REVISION)"'
