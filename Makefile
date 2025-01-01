# --------------------------------
# Makefile pro pdf_tool
# --------------------------------

CXX       = g++
CXXFLAGS  = -std=c++17 -Wall -Wextra
# Při reálném nasazení OCR: odkomentujte -ltesseract -llept
TESSERACT_LIBS = -ltesseract -llept

# Nastavení linkování pro PoDoFo (zkusíme pkg-config)
# Pokud pkg-config není k dispozici nebo nenašel podofo,
# můžete ručně uvést cesty -I/path -L/path -lpodofo.
PODOFO_LIBS = $(shell pkg-config --cflags --libs podofo)

TARGET    = pdf_tool
SOURCES   = pdf_tool.cpp
OBJECTS   = pdf_tool.o

.PHONY: all clean run

# --------------------------------
# Výchozí cíl (target)
# --------------------------------
all: $(TARGET)

# --------------------------------
# Linkování binárky z objektových souborů
# --------------------------------
$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) $(OBJECTS) $(PODOFO_LIBS) $(TESSERACT_LIBS) -o $(TARGET)

# --------------------------------
# Překlad zdrojového souboru do .o
# --------------------------------
$(OBJECTS): $(SOURCES)
	$(CXX) $(CXXFLAGS) $(PODOFO_LIBS) -c $(SOURCES) -o $(OBJECTS)

# --------------------------------
# make clean - smaže dočasné soubory
# --------------------------------
clean:
	rm -f $(OBJECTS) $(TARGET)

# --------------------------------
# Volitelný cíl: make run
# --------------------------------
run: $(TARGET)
	./$(TARGET) -h
