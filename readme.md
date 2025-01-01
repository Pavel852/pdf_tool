# pdf_tool

**Autor**: PB (pavel.bartos.pb@gmail.com)  
**Licence**: CC  

## Popis
Nástroj `pdf_tool` v jazyce C++ umožňuje:  
1. **Sloučení více PDF souborů** do jednoho.  
2. **Konverzi** obrázků (JPG, PNG) do PDF.  
3. **OCR** – extrakci textu z obrázků (JPG, PNG) nebo PDF do textového souboru (UTF-8).

## Parametry příkazové řádky (CLI)

- `-h, --help`  
  Zobrazí nápovědu.
  
- `-v, --version`  
  Zobrazí verzi a informace o autorovi.
  
- `-i`  
  Vstupní soubory (PDF, JPG, PNG). Lze zadat více souborů, např. `-i file1.pdf file2.pdf ...`
  
- `-o`  
  Výstupní soubor, např. `-o output.pdf` nebo `-o vysledek.txt` podle konkrétní funkce.
  
- `-f`  
  Určuje funkci:
  - `blend`   – sloučí PDF do jednoho souboru  
  - `convert` – konvertuje JPG/PNG do PDF  
  - `ocr`     – spustí OCR na zadané PDF/JPG/PNG a uloží text do .txt souboru
  
### Příklad použití:

1. Sloučení PDF:
./pdf_tool -f blend -i file1.pdf file2.pdf file3.pdf -o final.pdf

2. Konverze obrázku (JPG) do PDF:
./pdf_tool -f convert -i obrazek.jpg -o vystup.pdf

3. OCR z PDF nebo obrázku do textového souboru:
./pdf_tool -f ocr -i scan.pdf -o text_vystup.txt

## Kompilace

Základní kompilace bez externích knihoven:
g++ -std=c++17 pdf_tool.cpp -o pdf_tool


### Kompilace s Tesseract OCR
Pokud chcete, aby fungovala funkce OCR, je potřeba mít nainstalované knihovny Tesseract a Leptonica:  
Instalace knihoven v ubuntu: **apt install tesseract-ocr libtesseract-dev libleptonica-dev libpodofo-dev**

g++ -std=c++17 pdf_tool.cpp -o pdf_tool -ltesseract -llept


### Kompilace s knihovnou Poppler nebo PoDoFo
Pro sloučení PDF (funkce `blend`) a konverzi obrázků (funkce `convert`) do PDF můžete použít Poppler nebo PoDoFo (a další). Pak je nutné přilinkovat dané knihovny např. přes `pkg-config`. Příklad s Popplerem:
g++ -std=c++17 pdf_tool.cpp -o pdf_tool
$(pkg-config --cflags --libs poppler-cpp)
-ltesseract -llept


## Závislosti (doporučené balíčky)
- **Tesseract** (např. balíčky `tesseract-ocr`, `libtesseract-dev`, `libleptonica-dev`)  
- **Poppler** (např. balíčky `libpoppler-dev`, `libpoppler-cpp-dev`) nebo **PoDoFo** (`libpodofo-dev`)  
- Případně další knihovny dle potřeby pro PDF a OCR

## Licence
Tento kód je uvolněn pod licencí [Creative Commons (CC)](https://creativecommons.org/licenses/).  

