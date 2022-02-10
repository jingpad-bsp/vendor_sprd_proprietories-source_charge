ifneq ($(filter $(strip $(TARGET_BOOTLOADER_BOARD_NAME)), ud710_2h10 ud710_2h10u ud710_3h10u ud710_9h10u ud710_20c11 ud710_2c11),)
PRODUCT_COPY_FILES += vendor/sprd/proprietories-source/charge/charge.emcp_umcp.rc:vendor/etc/init/charge.rc
else ifneq ($(filter $(strip $(TARGET_BOARD_PLATFORM)),ums510 ums312 ums518 ums512 ums518-zebu ud710),)
PRODUCT_COPY_FILES += vendor/sprd/proprietories-source/charge/charge.new.rc:vendor/etc/init/charge.rc
else
PRODUCT_COPY_FILES += vendor/sprd/proprietories-source/charge/charge.rc:vendor/etc/init/charge.rc
endif
