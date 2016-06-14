# -*- Makefile -*-
# (C) Eugene Skepner 2016

# submodules and git: https://git-scm.com/book/en/v2/Git-Tools-Submodules

# ----------------------------------------------------------------------

MAKEFLAGS = -w

# ----------------------------------------------------------------------

CLANG = $(shell if g++ --version 2>&1 | grep -i llvm >/dev/null; then echo Y; else echo N; fi)
ifeq ($(CLANG),Y)
  WEVERYTHING = -Weverything -Wno-c++98-compat -Wno-c++98-compat-pedantic -Wno-padded
  WARNINGS = # -Wno-weak-vtables # -Wno-padded
  STD = c++14
else
  WEVERYTHING = -Wall -Wextra
  WARNINGS =
  STD = c++14
endif

# OPTIMIZATION = -O3
CXXFLAGS = -MMD -g $(OPTIMIZATION) -fPIC -std=$(STD) $(WEVERYTHING) $(WARNINGS) -Iaxe/include
LDFLAGS =
TEST_LDLIBS = # -L/usr/local/lib -lprofiler

# ----------------------------------------------------------------------

BUILD = build
DIST = dist

all: test

-include $(BUILD)/*.d

# ----------------------------------------------------------------------

test: $(DIST)/test
	gtime $(DIST)/test

$(DIST)/test: $(BUILD)/test.o | $(DIST)
	g++ $(LDFLAGS) -o $@ $^ $(TEST_LDLIBS)

clean:
	rm -rf $(DIST) $(BUILD)/*.o $(BUILD)/*.d

distclean: clean
	rm -rf $(BUILD)

# ----------------------------------------------------------------------

$(BUILD)/%.o: %.cc | $(BUILD)
	@#echo $<
	g++ $(CXXFLAGS) -c -o $@ $<

# ----------------------------------------------------------------------

$(DIST):
	mkdir -p $(DIST)

$(BUILD):
	mkdir -p $(BUILD)

# ======================================================================
