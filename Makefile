# Makefile pro pdf_tool (QPDF + Tesseract)
# ----------------------------------------
# Pouziti:
#   make        # zkompiluje pdf_tool
#   ./pdf_tool  # spustite program
#   make clean  # vycisti

CXX       := g++
CXXFLAGS  := -Wall -g -std=c++11

# QPDF a Tesseract knihovny
# Pokud máte nainstalováno v systémových cestách, stačí -lqpdf -ltesseract -llept
# (pokud byste potřebovali explicitní cesty: doplnit -I/usr/include/qpdf atd.)
LDFLAGS   := -lqpdf -ltesseract -llept

TARGET    := pdf_tool
SRCS      := pdf_tool.cpp
OBJS      := $(SRCS:.cpp=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean
