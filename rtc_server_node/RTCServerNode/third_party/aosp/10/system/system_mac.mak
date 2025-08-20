SO_PATH=../out

LIBRARY_NAME := libsystem.dylib

CFLAGS := -D_LARGEFILE64_SOURCE -DFAKE_LOG_DEVICE -DAUDIO_NO_SYSTEM_DECLARATIONS -D__unused="__attribute__((unused))"

CORE_BASE_INC := ./core/base/include

CORE_UTILS_INC := ./core/libutils/include

CORE_CUTILS_INC := ./core/libcutils/include

CORE_LOG_INC := ./core/liblog/include

CORE_PROCESS_GROUP_INC := ./core/libprocessgroup/include


MEDIA_AUDIO_UTILS_INC := ./media/audio_utils/include



CC = clang++ -O2
GCC = gcc -O2 -g

INC = -I$(CORE_BASE_INC)
INC += -I$(CORE_UTILS_INC)
INC += -I$(CORE_CUTILS_INC)
INC += -I$(CORE_LOG_INC)
INC += -I$(CORE_PROCESS_GROUP_INC)
INC += -I$(MEDIA_AUDIO_UTILS_INC)


CORE_BASE_SOURCE := \
	./core/base/chrono_utils.cpp \
	./core/base/cmsg.cpp \
	./core/base/errors_unix.cpp \
	./core/base/file.cpp \
	./core/base/logging.cpp \
	./core/base/mapped_file.cpp \
	./core/base/parsenetaddress.cpp \
	./core/base/quick_exit.cpp \
	./core/base/stringprintf.cpp \
	./core/base/strings.cpp \
	./core/base/threads.cpp

#	./core/libutils/Looper.cpp \

CORE_UTILS_SOURCE := \
	./core/libutils/FileMap.cpp \
	./core/libutils/JenkinsHash.cpp \
	./core/libutils/misc.cpp \
	./core/libutils/NativeHandle.cpp \
	./core/libutils/Printer.cpp \
	./core/libutils/PropertyMap.cpp \
	./core/libutils/RefBase.cpp \
	./core/libutils/SharedBuffer.cpp \
	./core/libutils/StopWatch.cpp \
	./core/libutils/String8.cpp \
	./core/libutils/String16.cpp \
	./core/libutils/StrongPointer.cpp \
	./core/libutils/SystemClock.cpp \
	./core/libutils/Threads.cpp \
	./core/libutils/Timers.cpp \
	./core/libutils/Tokenizer.cpp \
	./core/libutils/Unicode.cpp \
	./core/libutils/VectorImpl.cpp



CORE_CUTILS_SOURCE := \
	./core/libcutils/native_handle.cpp \
	./core/libcutils/sockets_unix.cpp \
	./core/libcutils/socket_network_client_unix.cpp \
	./core/libcutils/socket_local_server_unix.cpp \
	./core/libcutils/socket_local_client_unix.cpp \
	./core/libcutils/socket_inaddr_any_server_unix.cpp \
	./core/libcutils/sockets.cpp
		

CORE_LOG_SOURCE := \
	./core/liblog/config_read.cpp \
	./core/liblog/config_write.cpp \
	./core/liblog/fake_log_device.cpp \
	./core/liblog/fake_writer.cpp \
	./core/liblog/log_event_list.cpp \
	./core/liblog/log_event_write.cpp \
	./core/liblog/log_time.cpp \
	./core/liblog/logger_lock.cpp \
	./core/liblog/logger_name.cpp \
	./core/liblog/logger_read.cpp \
	./core/liblog/logger_write.cpp \
	./core/liblog/logprint.cpp \
	./core/liblog/pmsg_reader.cpp \
	./core/liblog/pmsg_writer.cpp \
	./core/liblog/stderr_write.cpp


CORE_PROCESS_GROUP_SOURCE := \
	./core/libprocessgroup/sched_policy.cpp


MEDIA_AUDIO_UTILS_SOURCE := \
	./media/audio_utils/primitives.c

	
CPP_SOURCE := $(CORE_BASE_SOURCE)
CPP_SOURCE += $(CORE_UTILS_SOURCE)
CPP_SOURCE += $(CORE_CUTILS_SOURCE)
CPP_SOURCE += $(CORE_LOG_SOURCE)
CPP_SOURCE += $(CORE_PROCESS_GROUP_SOURCE)


C_SOURCE := $(MEDIA_AUDIO_UTILS_SOURCE)

TARGET := $(SO_PATH)/$(LIBRARY_NAME)


CPP_OBJECTS := $(addsuffix .o, $(basename $(CPP_SOURCE)))

C_OBJECTS := $(addsuffix .o, $(basename $(C_SOURCE)))

.SUFFIXES : .o .c .cpp
%.o : %.c
	@echo ""
	@echo $< "=>" $@
	@$(GCC) -fPIC $(INC) -c $< -o $@


%.o : %.cpp
	@echo ""
	@echo $< "=>" $@
	@$(CC) -std=c++17 -fPIC $(CFLAGS) -DNDEBUG -Wno-multichar -fno-exceptions -fno-rtti -fno-use-cxa-atexit $(INC) -c $< -o $@


$(TARGET) : $(C_OBJECTS) $(CPP_OBJECTS)
	@echo $@ Linking...
	@$(CC) -dynamiclib -Wl,-install_name,$(LIBRARY_NAME) $^ -lpthread -o $@ 
#	@$(CC) -shared -stdlib=libc++ -Wl,-soname,$(LIBRARY_NAME) -Wl,-Bsymbolic $^ -lpthread -o $@ -Wl, --no-undefined
	@echo $@ build ok!
	
	

clean:
	rm -f $(TARGET) $(CPP_OBJECTS) $(C_OBJECTS)
	
	
	

	
	
	

