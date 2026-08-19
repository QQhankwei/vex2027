# VEXcode makefile 2019_03_26_01

# show compiler output
VERBOSE = 0

# include toolchain options
include vex/mkenv.mk

# location of the project source cpp and c files
SRC_C  = $(wildcard src/*.cpp) 
SRC_C += $(wildcard src/*.c)

# 正式使用完整 JAR-Template：PID、Drive、Odom 與共用工具只保留一套。
SRC_C += $(wildcard src/JAR-Template/*.cpp)

# JAR-Template 原始碼保留作為後續 PID / Odometry 教材。
# 基礎階段尚未接入，因此目前不加入編譯來源。

OBJ = $(addprefix $(BUILD)/, $(addsuffix .o, $(basename $(SRC_C))) )

# location of include files that c and cpp files depend on
SRC_H  = $(wildcard include/*.h)

# additional dependancies
SRC_A  = makefile

# project header file locations
INC_F  = include

# build targets
all: $(BUILD)/$(PROJECT).bin

# include build rules
include vex/mkrules.mk
