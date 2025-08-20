CORE_INC := -I../system/core/include


CC = clang++ -O2

#CC = g++ -O2

TEST_SOURCE := \
	./system_core_test.cpp

OUT_LIBS := -L../out

TARGET := system_core_test

CPP_OBJECTS := $(addsuffix .o, $(basename $(TEST_SOURCE)))

.SUFFIXES : .o .cpp
%.o : %.cpp
	@echo ""
	@echo $< "=>" $@
	@$(CC) -std=c++17 -DNDEBUG -fno-rtti $(CORE_INC) -c $< -o $@


$(TARGET) : $(CPP_OBJECTS)
	@echo $@ Linking...
	@$(CC) $(OUT_LIBS) $^ -lsystem -ldl -o $@
	@echo $@ build ok!


clean:
	rm -f $(TARGET) $(CPP_OBJECTS)