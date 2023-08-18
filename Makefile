CXXFLAGS += -O3 -Wall -shared -std=c++11 -fPIC -fdiagnostics-color
CPPFLAGS += $(shell python -m pybind11 --includes)
LIBNAME = text_filter
LIBEXT = $(shell python3-config --extension-suffix)

default: $(LIBNAME)$(LIBEXT)

%$(LIBEXT): %.cpp
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $< -o $@
