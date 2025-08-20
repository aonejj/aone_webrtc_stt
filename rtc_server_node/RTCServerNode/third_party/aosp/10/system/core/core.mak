SO_PATH=../../out

LIBRARY_NAME := libsystem_core.so

CFLAGS := -D_LARGEFILE64_SOURCE -DFAKE_LOG_DEVICE

BASE_INC := ./base/include

UTILS_INC := ./libutils/include

CUTILS_INC := ./libcutils/include

LOG_INC := ./liblog/include

PROCESS_GROUP_INC := ./libprocessgroup/include





CC = clang++ -O2


INC = -I$(BASE_INC)
INC += -I$(UTILS_INC)
INC += -I$(CUTILS_INC)
INC += -I$(LOG_INC)
INC += -I$(PROCESS_GROUP_INC)


BASE_SOURCE := \
	./base/chrono_utils.cpp \
	./base/cmsg.cpp \
	./base/errors_unix.cpp \
	./base/file.cpp \
	./base/logging.cpp \
	./base/mapped_file.cpp \
	./base/parsenetaddress.cpp \
	./base/quick_exit.cpp \
	./base/stringprintf.cpp \
	./base/strings.cpp \
	./base/threads.cpp



UTILS_SOURCE := \
	./libutils/FileMap.cpp \
	./libutils/JenkinsHash.cpp \
	./libutils/Looper.cpp \
	./libutils/misc.cpp \
	./libutils/NativeHandle.cpp \
	./libutils/Printer.cpp \
	./libutils/PropertyMap.cpp \
	./libutils/RefBase.cpp \
	./libutils/SharedBuffer.cpp \
	./libutils/StopWatch.cpp \
	./libutils/String8.cpp \
	./libutils/String16.cpp \
	./libutils/StrongPointer.cpp \
	./libutils/SystemClock.cpp \
	./libutils/Threads.cpp \
	./libutils/Timers.cpp \
	./libutils/Tokenizer.cpp \
	./libutils/Unicode.cpp \
	./libutils/VectorImpl.cpp



CUTILS_SOURCE := \
	./libcutils/native_handle.cpp \
	./libcutils/sockets_unix.cpp \
	./libcutils/socket_network_client_unix.cpp \
	./libcutils/socket_local_server_unix.cpp \
	./libcutils/socket_local_client_unix.cpp \
	./libcutils/socket_inaddr_any_server_unix.cpp \
	./libcutils/sockets.cpp
		

LOG_SOURCE := \
	./liblog/config_read.cpp \
	./liblog/config_write.cpp \
	./liblog/fake_log_device.cpp \
	./liblog/fake_writer.cpp \
	./liblog/log_event_list.cpp \
	./liblog/log_event_write.cpp \
	./liblog/log_time.cpp \
	./liblog/logger_lock.cpp \
	./liblog/logger_name.cpp \
	./liblog/logger_read.cpp \
	./liblog/logger_write.cpp \
	./liblog/logprint.cpp \
	./liblog/pmsg_reader.cpp \
	./liblog/pmsg_writer.cpp \
	./liblog/stderr_write.cpp


PROCESS_GROUP_SOURCE := \
	./libprocessgroup/sched_policy.cpp



	
CPP_SOURCE := $(BASE_SOURCE)
CPP_SOURCE += $(UTILS_SOURCE)
CPP_SOURCE += $(CUTILS_SOURCE)
CPP_SOURCE += $(LOG_SOURCE)
CPP_SOURCE += $(PROCESS_GROUP_SOURCE)


TARGET := $(SO_PATH)/$(LIBRARY_NAME)

CPP_OBJECTS := $(addsuffix .o, $(basename $(CPP_SOURCE)))

.SUFFIXES : .o .cpp
%.o : %.cpp
	@echo ""
	@echo $< "=>" $@
	@$(CC) -std=c++17 -fPIC $(CFLAGS) -DNDEBUG -Wno-multichar -fno-exceptions -fno-rtti -fno-use-cxa-atexit $(INC) -c $< -o $@


$(TARGET) : $(CPP_OBJECTS)
	@echo $@ Linking...
#	@$(CC) -stdlib=libc++ -Wl,-soname,$@ -Wl,-shared,-Bsymbolic $^ -lc++ -lc++abi -o $@ -Wl, --no-undefined			<== std::string_cxx11 이따구 링크에러 겁나남, stdc++ 교체
	@$(CC) -shared -stdlib=libstdc++ -Wl,-soname,$(LIBRARY_NAME) -Wl,-Bsymbolic $^ -lpthread -o $@ -Wl, --no-undefined
	@echo $@ build ok!
	
	

clean:
	rm -f $(TARGET) $(CPP_OBJECTS)
	
	
	

	
	
	

