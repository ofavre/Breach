# Define the main program name
PROG := breach

# Folder definitions
INCLUDE_DIR := include
SRC_DIR := src
TEST_DIR := test
BUILD_DIR := build
DIST_DIR := dist
DOC_DIR := doc

# Tools configuration
CXX := g++
CXX_FLAGS_COMPILATION := -c -Wall -Wextra
CXX_FLAGS_INCLUDE_BREACH := -I$(INCLUDE_DIR)
CXX_FLAGS_LIBS := `pkg-config --cflags sigc++-2.0`
CXX_FLAGS := $(CXX_FLAGS_COMPILATION) $(CXX_FLAGS_INCLUDE_BREACH) $(CXX_FLAGS_LIBS)
CXX_FLAGS_RELEASE := -g1 -O2
CXX_FLAGS_DEBUG := -g3 -O0
LN := g++
LN_FLAGS := 
LN_LIBS := -lm `pkg-config --libs glu` -lglut `libpng-config --libs` `pkg-config --libs sigc++-2.0`
LN_FLAGS_RELEASE := -g3
LN_FLAGS_DEBUG := -g3

# General extension definitions
PROG_EXT :=
PROG_EXT_DEBUG := _d
OBJ_EXT := o
OBJ_EXT_DEBUG := o.d

# Build final program file names
PROG_DEBUG := $(DIST_DIR)/$(PROG)$(PROG_EXT_DEBUG)$(PROG_EXT)
PROG := $(DIST_DIR)/$(PROG)$(PROG_EXT)

# List source file (and strip path)
SRC_FN := $(patsubst $(SRC_DIR)/%, %, $(wildcard $(SRC_DIR)/*.cpp))
# Derive object file names
OBJ_FN := $(patsubst %.cpp, %.$(OBJ_EXT), $(filter %.cpp,$(SRC_FN)))
OBJ_DEBUG_FN := $(patsubst %.cpp, %.$(OBJ_EXT_DEBUG), $(filter %.cpp,$(SRC_FN)))
# Construct final file names by prepending the folder path
SRC := $(addprefix $(SRC_DIR)/, $(SRC_FN))
OBJ := $(addprefix $(BUILD_DIR)/, $(OBJ_FN))
OBJ_DEBUG := $(addprefix $(BUILD_DIR)/, $(OBJ_DEBUG_FN))

# Template defining header dependencies for a source file
define TEMPLATE_SOURCE_HEADER_DEPENDENCIES
# We have to strip the slashes and newlines, because they are taken for escaped spaces and count for a dependency!
$(eval $(shell $(CXX) $(CXX_FLAGS_INCLUDE_BREACH) -MM $(SRC_DIR)/$(1).cpp -MT $(BUILD_DIR)/$(1).$(OBJ_EXT) -MT $(BUILD_DIR)/$(1).$(OBJ_EXT_DEBUG) | tr -d '\\\n'))
endef
# Each source file depends on its include dependencies
$(foreach src_fn,$(patsubst %.cpp,%,$(SRC_FN)), $(eval $(call TEMPLATE_SOURCE_HEADER_DEPENDENCIES,$(src_fn))))

# Same story with tests, plus define the programs file names
# (from each single test source file will derive a single test program)
TEST_SRC := $(patsubst $(TEST_DIR)/$(SRC_DIR)/%, %, $(wildcard $(TEST_DIR)/$(SRC_DIR)/*.cpp))
TEST_OBJ := $(patsubst %.cpp, %.$(OBJ_EXT), $(filter %.cpp,$(TEST_SRC)))
TEST_OBJ_DEBUG := $(patsubst %.cpp, %.$(OBJ_EXT_DEBUG), $(filter %.cpp,$(TEST_SRC)))
TEST_PROG := $(patsubst %.cpp, %$(PROG_EXT), $(filter %.cpp,$(TEST_SRC)))
TEST_PROG_DEBUG := $(patsubst %.cpp, %$(PROG_EXT_DEBUG)$(PROG_EXT), $(filter %.cpp,$(TEST_SRC)))
TEST_SRC := $(addprefix $(TEST_DIR)/$(SRC_DIR)/, $(TEST_SRC))
TEST_OBJ := $(addprefix $(TEST_DIR)/$(BUILD_DIR)/, $(TEST_OBJ))
TEST_OBJ_DEBUG := $(addprefix $(TEST_DIR)/$(BUILD_DIR)/, $(TEST_OBJ_DEBUG))
TEST_PROG := $(addprefix $(TEST_DIR)/$(DIST_DIR)/, $(TEST_PROG))
TEST_PROG_DEBUG := $(addprefix $(TEST_DIR)/$(DIST_DIR)/, $(TEST_PROG_DEBUG))

# Template defining targets for running a particular test (given as argument)
define TEMPLATE_RUN_TEST
.PHONY: RUN_TEST_$(1)
RUN_TEST_$(1): $(1)
	./$(1)
endef
# Define the targets that permit running tests
$(foreach test,$(TEST_PROG),$(eval $(call TEMPLATE_RUN_TEST,$(test))))
$(foreach test,$(TEST_PROG_DEBUG),$(eval $(call TEMPLATE_RUN_TEST,$(test))))



# General make targets configuration
.DEFAULT_GOAL = all
.PHONY: all doc compile compile-debug compile-test compile-test-debug run debug gdb test test-debug clean dist-clean
.SECONDARY: $(OBJ) $(OBJ_DEBUG) $(TEST_OBJ) $(TEST_OBJ_DEBUG)



#
# All targets of this makefile
#

# Default target
all: compile doc

# Create the documentation
doc: Doxyfile
	doxygen $^
	# Invoke the creation of the PDF documentation (not automatic)
	-make -C doc/latex refman.pdf

# Compile targets
compile: $(PROG)

compile-debug: $(PROG_DEBUG)

compile-test: $(TEST_PROG)

compile-test-debug: $(TEST_PROG_DEBUG)

# Running targets
run: compile
	$(PROG)

debug: compile-debug
	$(PROG_DEBUG) -gldebug

gdb: compile-debug
	gdb $(PROG_DEBUG)

test: compile-test $(foreach test,$(TEST_PROG),RUN_TEST_$(test))

test-debug: compile-test-debug $(foreach test,$(TEST_PROG_DEBUG),RUN_TEST_$(test))

# Householding targets
clean:
	rm -f $(OBJ) $(OBJ_DEBUG) $(PROG) $(PROG_DEBUG) $(TEST_OBJ) $(TEST_OBJ_DEBUG) $(TEST_PROG) $(TEST_PROG_DEBUG)

dist-clean: clean
	rm -Rf $(DIST_DIR) $(BUILD_DIR) $(TEST_DIR)/$(DIST_DIR) $(TEST_DIR)/$(BUILD_DIR) $(DOC_DIR)
	find -name '*~' -exec rm -f {} \;



#
# File targets
#

# Ensure creation of necessary folders
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)
$(DIST_DIR):
	mkdir -p $(DIST_DIR)
$(TEST_DIR)/$(BUILD_DIR):
	mkdir -p $(TEST_DIR)/$(BUILD_DIR)
$(TEST_DIR)/$(DIST_DIR):
	mkdir -p $(TEST_DIR)/$(DIST_DIR)



# Compilation of the main program
$(PROG): $(OBJ) | $(DIST_DIR)
	$(LN) $(LN_FLAGS) $(LN_FLAGS_RELEASE) $(LN_LIBS) -o $@ $^

$(PROG_DEBUG): $(OBJ_DEBUG) | $(DIST_DIR)
	$(LN) $(LN_FLAGS) $(LN_FLAGS_DEBUG) $(LN_LIBS) -o $@ $^

# Compilation of each test program
$(TEST_DIR)/$(DIST_DIR)/%$(PROG_EXT): $(TEST_DIR)/$(BUILD_DIR)/%.$(OBJ_EXT) | $(TEST_DIR)/$(DIST_DIR)
	$(LN) $(LN_FLAGS) $(LN_LIBS) -o $@ $^

$(TEST_DIR)/$(DIST_DIR)/%$(PROG_EXT_DEBUG)$(PROG_EXT): $(TEST_DIR)/$(BUILD_DIR)/%.$(OBJ_EXT_DEBUG) | $(TEST_DIR)/$(DIST_DIR)
	$(LN) $(LN_FLAGS) $(LN_LIBS) -o $@ $^


# Object creation for the main program
$(BUILD_DIR)/%.$(OBJ_EXT): $(SRC_DIR)/%.cpp | $(BUILD_DIR)
	$(CXX) $(CXX_FLAGS) $(CXX_FLAGS_RELEASE) -o $@ $<

$(BUILD_DIR)/%.$(OBJ_EXT_DEBUG): $(SRC_DIR)/%.cpp | $(BUILD_DIR)
	$(CXX) $(CXX_FLAGS) $(CXX_FLAGS_DEBUG) -o $@ $<

# Object creation for the test programs
$(TEST_DIR)/$(BUILD_DIR)/%.$(OBJ_EXT): $(TEST_DIR)/$(SRC_DIR)/%.cpp | $(TEST_DIR)/$(BUILD_DIR)
	$(CXX) $(CXX_FLAGS) $(CXX_FLAGS_RELEASE) -o $@ $<

$(TEST_DIR)/$(BUILD_DIR)/%.$(OBJ_EXT_DEBUG): $(TEST_DIR)/$(SRC_DIR)/%.cpp | $(TEST_DIR)/$(BUILD_DIR)
	$(CXX) $(CXX_FLAGS) $(CXX_FLAGS_DEBUG) -o $@ $<
