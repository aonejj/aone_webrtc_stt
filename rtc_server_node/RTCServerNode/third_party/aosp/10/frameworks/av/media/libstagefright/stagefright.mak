SO_PATH = ../../../../out

LIBRARY_NAME := libstagefright.so

CFLAGS := -DNO_IMEMORY -D__unused="__attribute__((unused))"

AV_INC := ../../include

FOUNDATION_INC := ../../include/media/stagefright/foundation

NATIVE_OMX_MEDIA_INC := ../../../native/headers/media_plugin

SYSTEM_CORE_INC :=../../../../system/core/include

SYSTEM_AUDIO_INC := ../../../../system/media/audio/include

SYSTEM_AUDIO_UTILS_INC := ../../../../system/media/audio_utils/include

NDK_INC := ../ndk/include



INC = -I$(SYSTEM_CORE_INC)
INC += -I$(SYSTEM_AUDIO_INC)
INC += -I$(SYSTEM_AUDIO_UTILS_INC)
INC += -I$(AV_INC)
INC += -I$(FOUNDATION_INC)
INC += -I$(NATIVE_OMX_MEDIA_INC)
INC += -I$(NDK_INC)

OUT_LIBS := -L../../../../out

CXX = clang++ -O2

FOUNDATION_SOURCE := \
	./foundation/AAtomizer.cpp \
	./foundation/ABitReader.cpp \
	./foundation/ABuffer.cpp \
	./foundation/ADebug.cpp \
	./foundation/AHandler.cpp \
	./foundation/ALooper.cpp \
	./foundation/ALooperRoster.cpp \
	./foundation/AMessage.cpp \
	./foundation/AString.cpp \
	./foundation/AStringUtils.cpp \
	./foundation/AudioPresentationInfo.cpp \
	./foundation/avc_utils.cpp \
	./foundation/base64.cpp \
	./foundation/ByteUtils.cpp \
	./foundation/hexdump.cpp \
	./foundation/MediaBuffer.cpp \
	./foundation/MediaBufferBase.cpp \
	./foundation/MediaBufferGroup.cpp \
	./foundation/MediaDefs.cpp \
	./foundation/MediaKeys.cpp \
	./foundation/MetaData.cpp \
	./foundation/MetaDataBase.cpp \
	./foundation/OpusHeader.cpp


MEDIA_SOURCE := \
	../libmedia/NdkMediaErrorPriv.cpp	\
	../libmedia/NdkMediaFormatPriv.cpp	\
	../libmedia/BufferingSettings.cpp	\
	../libmedia/MediaCodecBuffer.cpp



STAGEFRIGHT_SOURCE := \
	./MediaTrack.cpp \
	./MediaSource.cpp \
	./Utils.cpp \
	./ESDS.cpp  \
	./DataConverter.cpp \
	./HevcUtils.cpp





CPP_SOURCE := $(FOUNDATION_SOURCE)
CPP_SOURCE += $(MEDIA_SOURCE)
CPP_SOURCE += $(STAGEFRIGHT_SOURCE)

TARGET := $(SO_PATH)/$(LIBRARY_NAME)

CPP_OBJECTS := $(addsuffix .o, $(basename $(CPP_SOURCE)))


.SUFFIXES : .o .cpp
%.o : %.cpp
	@echo ""
	@echo $< "=>" $@
	@$(CXX) -std=c++17 -fPIC $(CFLAGS) -DNDEBUG -Wno-multichar -fno-exceptions -fno-rtti -fno-use-cxa-atexit $(INC) -c $< -o $@

$(TARGET) : $(CPP_OBJECTS)
	@echo $@ Linking...
	@$(CXX) $(OUT_LIBS) -shared -Wl,-soname,$(LIBRARY_NAME) -Wl,-Bsymbolic $^ -lsystem -lpthread -o $@ -Wl, --no-undefined
#	@$(CC) $(OUT_LIBS) -shared -stdlib=libstdc++ -Wl,-soname,$(LIBRARY_NAME) -Wl,-Bsymbolic $^ -lsystem -lpthread -o $@ -Wl, --no-undefined
	@echo $@ build ok!


clean:
	rm -f $(TARGET) $(CPP_OBJECTS)
	

