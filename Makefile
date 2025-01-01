# --------------------------------
# Makefile pro pdf_tool (bez pkg-config)
# --------------------------------

CXX       = g++
CXXFLAGS  = -std=c++17 -Wall -Wextra
# Cesty k hlavičkám a knihovnám PoDoFo (upravte dle svého systému!)
PODOFO_INCLUDES = -I/usr/include/podofo
PODOFO_LIBS     = -L/usr/lib -lpodofo

# Pokud potřebujete Tesseract, odkomentujte (a musíte mít nainstalované)
# TESSERACT_LIBS = -ltesseract -llept

TARGET    = pdf_tool
SOURCES   = pdf_tool.cpp
OBJECTS   = pdf_tool.o

.PHONY: all clean run

# --------------------------------
# Výchozí cíl (target)
# --------------------------------
all: $(TARGET)

# --------------------------------
# Linkování binárky
# --------------------------------
$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) $(OBJECTS) $(PODOFO_LIBS) $(TESSERACT_LIBS) -o $(TARGET)

# --------------------------------
# Překlad zdrojového souboru do objektu
# --------------------------------
$(OBJECTS): $(SOURCES)
	$(CXX) $(CXXFLAGS) $(PODOFO_INCLUDES) -c $(SOURCES) -o $(OBJECTS)

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
