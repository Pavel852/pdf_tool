# -------------------------------
# Makefile pro pdf_tool
# -------------------------------

# Proměnné
CXX      = g++
CXXFLAGS = -std=c++17 -Wall -Wextra
LDFLAGS  = -ltesseract -llept  # Sem patří knihovny pro linkování (OCR)
TARGET   = pdf_tool
SRCS     = pdf_tool.cpp
OBJS     = pdf_tool.o

# Výchozí (default) cíl (target) - pokud do příkazu make nezadáte nic jiného
all: $(TARGET)

# Pravidlo pro sestavení spustitelného souboru
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $(TARGET) $(LDFLAGS)

# Pravidlo pro překlad zdrojového souboru do objektového
$(OBJS): $(SRCS)
	$(CXX) $(CXXFLAGS) -c $(SRCS) -o $(OBJS)

# "clean" smaže dočasné (objektové) i výsledný binární soubor
clean:
	rm -f $(OBJS) $(TARGET)

# Vlastní volitelný target pro úplné "distclean"
# (pokud byste měli např. generované soubory, dokumentaci atd.)
distclean: clean
	@echo "Zde byste mohli smazat i další generované výstupy..."

# Užití:
# - make        => zkompiluje projekt do binárky "pdf_tool"
# - make clean  => smaže objektové soubory a binárku
# - make run    => (pokud si tento target definujete) spustí program
