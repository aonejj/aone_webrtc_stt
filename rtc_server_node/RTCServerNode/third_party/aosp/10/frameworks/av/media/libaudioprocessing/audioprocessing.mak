SO_PATH = ../../../../out

LIBRARY_NAME := libaudioprocessing.so

CFLAGS := -DAUDIO_NO_SYSTEM_DECLARATIONS -D__unused="__attribute__((unused))"

SYSTEM_CORE_INC := ../../../../system/core/include

SYSTEM_AUDIO_INC := ../../../../system/media/audio/include

SYSTEM_AUDIO_UTILS_INC := ../../../../system/media/audio_utils/include

MEDIA_AUDIO_PROCESSING_INC := ./include

MEDIA_INC := ../../include


INC = -I$(SYSTEM_CORE_INC)
INC += -I$(SYSTEM_AUDIO_INC)
INC += -I$(SYSTEM_AUDIO_UTILS_INC)
INC += -I$(MEDIA_AUDIO_PROCESSING_INC)
INC += -I$(MEDIA_INC)

OUT_LIBS := -L../../../../out

CXX = clang++ -O2

AUDIO_PROCESSING_SOURCE := \
	./AudioResampler.cpp \
	./AudioResamplerCubic.cpp \
	./AudioResamplerDyn.cpp \
	./AudioResamplerSinc.cpp


CPP_SOURCE := $(AUDIO_PROCESSING_SOURCE)

TARGET := $(SO_PATH)/$(LIBRARY_NAME)

CPP_OBJECTS := $(addsuffix .o, $(basename $(CPP_SOURCE)))

.SUFFIXES : .o .cpp
%.o : %.cpp
	@echo ""
	@echo $< "=>" $@
	@$(CXX) -std=c++17 -fPIC $(CFLAGS) -DNDEBUG -Wno-multichar -fno-exceptions -fno-rtti -fno-use-cxa-atexit $(INC) -c $< -o $@

$(TARGET) : $(CPP_OBJECTS)
	@echo $@ Linking...
	@$(CXX) $(OUT_LIBS) -shared -Wl,-soname,$(LIBRARY_NAME) -Wl,-Bsymbolic $^ -lsystem -lpthread -ldl -o $@ -Wl, --no-undefined
#	@$(CC) $(OUT_LIBS) -shared -stdlib=libstdc++ -Wl,-soname,$(LIBRARY_NAME) -Wl,-Bsymbolic $^ -lsystem -lpthread -ldl -o $@ -Wl, --no-undefined
	@echo $@ build ok!


clean:
	rm -f $(TARGET) $(CPP_OBJECTS)