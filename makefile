#---------------------------------------------------------------------------------
# 条件判断句用ifeq ifneq endif else几个关键字
# error关键字用于显示错误，并终止脚本
#---------------------------------------------------------------------------------
ifeq ($(strip $(DEVKITARM)),)
$(error "请将DEVKITARM添加到环境变量")
endif

#---------------------------------------------------------------------------------
# 定义工具的路径
#---------------------------------------------------------------------------------

PREFIX	:=	$(DEVKITARM)\bin\arm-none-eabi-
CC	:=	$(PREFIX)gcc
OBJCOPY	:=	$(PREFIX)objcopy
LD	:=	$(PREFIX)ld

#---------------------------------------------------------------------------------
# 定义生成的相关参数
#---------------------------------------------------------------------------------

TARGET	:=	code
SOURCE	:=	source

#---------------------------------------------------------------------------------
# 定义编译代码时的各种参数
# 具体请参考GCC官方的手册http://gcc.gnu.org/onlinedocs/
#---------------------------------------------------------------------------------

CFLAGS	:=	-I $(SOURCE) -std=c99 -w -nodefaultlibs -O2 -mthumb -mthumb-interwork -mtune=arm7tdmi
LDFLAGS	:=	-T $(CURDIR)/symbol.sym -T $(CURDIR)/linker.ld
ifdef CODEADDR
  LDFLAGS += -Ttext $(CODEADDR)
endif

#---------------------------------------------------------------------------------
# 定义生成代码过程中的各种路径
# foreach做遍历，wildcard使后面的通配符生效
# 如果要添加额外的lib跟include文件，可以在INCLUDE跟LIBS后面添加
# lib的路径前加上"-L"，include文件前面添加"-I"
#---------------------------------------------------------------------------------

OUTPUT	:=	$(CURDIR)/$(TARGET)

CFILES	:=	$(foreach dir,$(SOURCE),$(wildcard $(dir)/*.c))
SFILES	:=	$(foreach dir,$(SOURCE),$(wildcard $(dir)/*.s))
SsFILES	:=	$(foreach dir,$(SOURCE),$(wildcard $(dir)/*.S))
OFILES	:=	$(CFILES:.c=.o) $(SFILES:.s=.o) $(SsFILES:.S=.o)

INCLUDE	:=	
LIBS	:=	lib_a-memmove.o

#---------------------------------------------------------------------------------
# 定义生成事件
# 先编译再链接，最后输出为bin文件
#---------------------------------------------------------------------------------

all: $(OUTPUT).bin

$(OUTPUT).bin : $(OUTPUT).elf
	@echo 正在生成 $(notdir $@)
	$(OBJCOPY) -O binary $< $@

%.elf: $(OFILES)
	@echo 正在链接 $(notdir $@)
	$(LD) $(LDFLAGS) $(LIBS) $(OFILES) -o $@

%.o: %.S
	@echo 正在编译 $(notdir $<)
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.s
	@echo 正在编译 $(notdir $<)
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.c
	@echo 正在编译 $(notdir $<)
	$(CC) $(CFLAGS) -c $< -o $@
