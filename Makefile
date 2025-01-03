# Makefile pro pdf_tool (PDFium + Tesseract)
# ------------------------------------------
# Předpokládáme, že v aktuálním adresáři je soubor pdf_tool.cpp
# a v systému nainstalované:
#   build-essential, libpdfium-dev, libtesseract-dev, libleptonica-dev
#
# Použití:
#   make          # zkompiluje pdf_tool
#   ./pdf_tool -h # spuštění programu
#   make clean    # vyčištění projektu

CXX       := g++
CXXFLAGS  := -Wall -g -std=c++11
LDFLAGS   := -lpdfium -ltesseract -llept

TARGET    := pdf_tool
SRCS      := pdf_tool.cpp
OBJS      := $(SRCS:.cpp=.o)

# Výchozí cíl
all: $(TARGET)

# Sestavení spustitelného souboru
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

# Kompilace zdrojových souborů .cpp na objektové .o
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Vyčištění
clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean
