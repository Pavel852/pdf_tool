========================================
 pdf_tool
========================================
Version: 1.0
Author:  PB, pavel.bartos.pb@gmail.com
License: CC

Popis:
-------
pdf_tool je nástroj příkazové řádky (CLI), který umožňuje:
1. Sloučit PDF a/nebo obrázky (PNG/JPG) do jednoho PDF (funkce "blend").
2. Provest OCR z PDF/PNG/JPG a uložit jako text (funkce "ocr").

Knihovny:
----------
- V závislosti na konkrétní implementaci se používají:
  - QPDF (nebo jiná PDF knihovna) pro práci s PDF.
  - Tesseract OCR pro rozpoznávání textu z obrázků.
  - (Dříve se používalo PDFium, Poppler, PoDoFo atp.)

Instalace závislostí (Ubuntu příklad):
---------------------------------------
sudo apt-get update
sudo apt-get install -y build-essential libqpdf-dev libtesseract-dev libleptonica-dev

Pokud chcete použít skutečný převod obrázků na PDF, nainstalujte také ImageMagick:
sudo apt-get install -y imagemagick

Kompilace:
-----------
1. V kořenovém adresáři projektu se souborem pdf_tool.cpp a Makefile spusťte:
   make
2. Vznikne binární soubor pdf_tool (spustitelný program).

Použití z příkazové řádky:
---------------------------
./pdf_tool -h
  - zobrazí nápovědu.

./pdf_tool -v
  - zobrazí verzi programu.

./pdf_tool -f=blend -i soubor1.pdf soubor2.jpg soubor3.png -o vysledek.pdf
  - sloučí zadané soubory do jednoho PDF (popř. konvertuje obrázky do PDF).

./pdf_tool -f=ocr -i dokument.pdf obrazek.jpg -o vysledek.txt
  - provede OCR a uloží rozpoznaný text do 'vysledek.txt'.

Chybové stavy:
---------------
- Pokud chybí -f=..., nástroj vypíše chybu "[ERROR] Nebyl zadan parametr -f=blend|ocr".
- Pokud chybí vstupní soubory (-i), dojde k chybě "[ERROR] Nebyly zadany vstupni soubory pomoci -i".
- Pokud není zadán výstup (-o), nástroj použije výchozí názvy (např. "output.pdf" nebo "output.txt").

Další poznámky:
----------------
- Pro reálné nasazení nezapomeňte řešit mazání dočasných souborů (pokud se vytvářejí).
- Pro OCR v jiném jazyce než "eng" je potřeba změnit inicializaci Tesseractu (např. "ces" pro češtinu) a mít nainstalované jazykové balíčky.

Kontakt a licence:
-------------------
- Autor: PB, pavel.bartos.pb@gmail.com
- License: Creative Commons (CC)
