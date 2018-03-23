# General Makefile for the project
CXX = g++
CFLAG = -std=c++17 -m32 --static -static-libstdc++ -DNDEBUG -mwindows -pthread -s -O3 -DUNICODE -D_UNICODE
CFLAG_D = -std=c++17 -g -Wall -m32
CFLAG64 = -std=c++17 -m64 --static -static-libstdc++ -DNDEBUG -mwindows -pthread -s -O3 -DUNICODE -D_UNICODE
CFLAG64_D = -std=c++17 -g -Wall -m64
LINKS := $(file < links.txt)
LIBRARIES := $(file < libraries.txt)
LINKS_D := $(file < links_d.txt)
LIBRARIES_D := $(file < libraries_d.txt)
LINKS64 := $(file < links64.txt)
LIBRARIES64 := $(file < libraries64.txt)
LINKS64_D := $(file < links64_d.txt)
LIBRARIES64_D := $(file < libraries64_d.txt)
CPPFILES := $(wildcard *.cpp) $(wildcard */*.cpp)
OBJFILES := $(addprefix obj/, $(CPPFILES:.cpp=.o)) obj/resource.o
DIRTOCREATE := out/ obj/ out/debug32/ out/debug64/ out/release32/ out/release64/ $(dir $(OBJFILES))
CURR_BUILDNO := $(file < buildNumber.txt)
BUILDNUMBER := $(shell expr $(CURR_BUILDNO) + 1)

FINAL_LINKS := 
FINAL_LIBRARIES := 
FINAL_FLAGS := 

$(shell mkdir -p $(DIRTOCREATE))
$(file > buildNumber.txt,$(BUILDNUMBER))

.PHONY: clean
.SILENT: clean

main:
	@echo "Re-run make with an argument: debug32(/64) or release32(/64) or clean"
	@echo "Target $@ ran."

debug32: FINAL_LINKS := $(LINKS_D)
debug32: FINAL_LIBRARIES := $(LIBRARIES_D)
debug32: FINAL_FLAGS := $(CFLAG_D)
debug32: out/debug32/wa.exe

release32: FINAL_LINKS := $(LINKS)
release32: FINAL_LIBRARIES := $(LIBRARIES)
release32: FINAL_FLAGS := $(CFLAG)
release32: out/release32/wa.exe

debug64: FINAL_LINKS := $(LINKS64_D)
debug64: FINAL_LIBRARIES := $(LIBRARIES64_D)
debug64: FINAL_FLAGS := $(CFLAG64_D)
debug64: out/debug64/wa.exe

release64: FINAL_LINKS := $(LINKS64)
release64: FINAL_LIBRARIES := $(LIBRARIES64)
release64: FINAL_FLAGS := $(CFLAG64)
release64: out/release64/wa.exe

%/wa.exe: $(OBJFILES)
	$(CXX) $(FINAL_FLAGS) -o $@ $^ $(FINAL_LINKS)
	mv obj $(dir $@)obj
	@echo Build Complete.

obj/%.o: %.cpp
	$(CXX) $(FINAL_FLAGS) -o $@ -c $^ $(FINAL_LIBRARIES) -DBUILDNUMBER=$(BUILDNUMBER)

obj/resource.o: resource.rc
	windres -i $< -o $@

clean:
	$(file > buildNumber.txt,$(CURR_BUILDNO))
	$(RM) -r out/ obj/
