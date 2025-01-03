# Makefile pro pdf_tool (PDFium + Tesseract)
# -------------------------------------------
# Příklad použití:
#   $ make
#   $ ./pdf_tool -h
#
# Pokud potřebujete vyčistit:
#   $ make clean
#

# Kompilátor a základní volby
CXX       := g++
CXXFLAGS  := -Wall -g -std=c++11

# Zde si můžete upravit cesty k hlavičkám a knihovnám PDFium / Tesseract
PDFIUM_INC := -I/opt/pdfium/include
PDFIUM_LIB := -L/opt/pdfium/lib -lpdfium

TESS_INC   := -I/usr/include/tesseract
TESS_LIB   := -ltesseract -llept

# Název výsledného spustitelného souboru
TARGET := pdf_tool

# Zdrojové soubory
SRCS   := pdf_tool.cpp

# Z objektových souborů (každý .cpp -> .o)
OBJS   := $(SRCS:.cpp=.o)

# Cílové pravidlo (all)
all: $(TARGET)

# Postup, jak sestavit finální binárku
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(PDFIUM_LIB) $(TESS_LIB)

# Pravidlo pro kompilaci .cpp -> .o
%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(PDFIUM_INC) $(TESS_INC) -c $< -o $@

# Vyčištění projektu
clean:
	rm -f $(OBJS) $(TARGET)
