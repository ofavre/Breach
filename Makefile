SRC_DIR := src
BUILD_DIR := build
DIST_DIR := dist
DOC_DIR := doc

CXX := g++
CXX_FLAGS := -c -g1 -Wall -Wextra -O2
CXX_DEBUG_FLAGS := -g3 -O0
LN := g++
LN_FLAGS := 
LN_LIBS := -lm `pkg-config --libs glu` -lglut `libpng-config --libs`
LN_DEBUG_FLAGS := -g3

PROG := breach
PROG_EXT :=
PROG_EXT_DEBUG := _d
PROG_DEBUG := $(DIST_DIR)/$(PROG)$(PROG_EXT_DEBUG)$(PROG_EXT)
PROG := $(DIST_DIR)/$(PROG)$(PROG_EXT)

SRC := main.cpp PngImage.cpp
OBJ_EXT := o
OBJ_EXT_DEBUG := o.d
OBJ := $(patsubst %.cpp, %.$(OBJ_EXT), $(filter %.cpp,$(SRC)))
OBJ_DEBUG := $(patsubst %.cpp, %.$(OBJ_EXT_DEBUG), $(filter %.cpp,$(SRC)))

SRC := $(addprefix $(SRC_DIR)/, $(SRC))
OBJ := $(addprefix $(BUILD_DIR)/, $(OBJ))
OBJ_DEBUG := $(addprefix $(BUILD_DIR)/, $(OBJ_DEBUG))



.PHONY: all doc compile compile-debug compile-test-matrix run debug gdb test-matrix clean distclean

all: compile compile-test-matrix doc
compile: $(PROG)
compile-debug: $(PROG_DEBUG)
compile-test-matrix: $(DIST_DIR)/matrix_test$(PROG_EXT)
run: compile
	$(PROG)
debug: compile-debug
	$(PROG_DEBUG)
gdb: compile-debug
	gdb $(PROG_DEBUG)
test-matrix: $(DIST_DIR)/matrix_test$(PROG_EXT)
	$(DIST_DIR)/matrix_test$(PROG_EXT)
clean:
	rm -f $(OBJ) $(OBJ_DEBUG) $(PROG) $(PROG_DEBUG) $(DIST_DIR)/matrix_test$(PROG_EXT) $(BUILD_DIR)/matrix_test.$(OBJ_EXT)
distclean:
	rm -f $(OBJ) $(OBJ_DEBUG) $(PROG) $(PROG_DEBUG)
	rm -Rf $(DIST_DIR) $(BUILD_DIR) $(DOC_DIR)
	find -name '*~' -exec rm -f {} \;
doc: Doxyfile
	doxygen $^
	-make -C doc/latex refman.pdf


$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)
$(DIST_DIR):
	mkdir -p $(DIST_DIR)



$(PROG): $(OBJ) | $(DIST_DIR)
	$(LN) $(LN_FLAGS) $(LN_LIBS) -o $@ $^

$(PROG_DEBUG): $(OBJ_DEBUG) | $(DIST_DIR)
	$(LN) $(LN_FLAGS) $(LN_FLAGS_DEBUG) $(LN_LIBS) -o $@ $^

$(DIST_DIR)/matrix_test$(PROG_EXT): $(BUILD_DIR)/matrix_test.$(OBJ_EXT) | $(DIST_DIR)
	$(LN) $(LN_FLAGS) $(LN_LIBS) -o $@ $^



$(BUILD_DIR)/%.$(OBJ_EXT): $(SRC_DIR)/%.cpp | $(BUILD_DIR)
	$(CXX) $(CXX_FLAGS) -o $@ $<

$(BUILD_DIR)/%.$(OBJ_EXT_DEBUG): $(SRC_DIR)/%.cpp | $(BUILD_DIR)
	$(CXX) $(CXX_FLAGS) $(CXX_DEBUG_FLAGS) -o $@ $<
