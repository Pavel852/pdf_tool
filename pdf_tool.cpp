/*
 * pdf_tool
 * Autor: PB (pavel.bartos.pb@gmail.com)
 * Licence: CC
 *
 * Implementace funkcí:
 * 1) Slouèení PDF souborù do jednoho (blend)  -- pomocí PoDoFo
 * 2) Konverze JPG/PNG do PDF (convert)        -- pomocí PoDoFo
 * 3) OCR (jpg/png/pdf do .txt)                -- pseudo-kód pro Tesseract
 */

#include <iostream>
#include <string>
#include <vector>
#include <fstream>

// -------------- PoDoFo --------------
#include <podofo/podofo.h>

// -------------- Tesseract OCR (pokud byste chtìli reálnì použít) --------------
#include <tesseract/baseapi.h>
#include <leptonica/allheaders.h>


// Pomocná funkce pro zobrazení nápovìdy (help)
void printHelp() {
    std::cout << "Pouziti: pdf_tool [parametry]\n"
              << "Parametry:\n"
              << "  -h, --help       Zobrazi tuto napovedu.\n"
              << "  -v, --version    Zobrazi verzi a informace o autorovi.\n"
              << "  -i <soubory>     Vstupni soubory (PDF, JPG, PNG). Lze zadat vice.\n"
              << "  -o <soubor>      Vystupni soubor.\n"
              << "  -f <funkce>      Funkce:\n"
              << "                   blend   - Slouceni PDF do jednoho\n"
              << "                   convert - Konverze JPG/PNG do PDF\n"
              << "                   ocr     - OCR: Extrakce textu z JPG/PNG/PDF do .txt\n"
              << "\nPriklad:\n"
              << "  pdf_tool -f blend   -i file1.pdf file2.pdf -o output.pdf\n"
              << "  pdf_tool -f convert -i obrazek.jpg -o vystup.pdf\n"
              << "  pdf_tool -f ocr     -i scan.pdf -o vysledek.txt\n";
}

// Pomocná funkce pro zobrazení verze
void printVersion() {
    std::cout << "pdf_tool v1.5\n"
              << "Autor: PB, pavel.bartos.pb@gmail.com\n"
              << "License: CC\n";
}

/*
 * Funkce pro slouèení PDF (blend) pomocí knihovny PoDoFo
 */
bool mergePDFs(const std::vector<std::string>& inputFiles, const std::string& outputFile) {
    using namespace PoDoFo;

    try {
        std::cout << "[INFO] Sluèuji PDF pomocí PoDoFo...\n";

        PdfMemDocument pdfOut;

        // Pro každý vstupní soubor
        for (const auto &inputPath : inputFiles) {
            // Naèteme PDF do doèasného dokumentu
            PdfMemDocument pdfIn;
            pdfIn.Load(inputPath.c_str());

            // Zjistíme poèet stránek
            int pageCount = pdfIn.GetPageCount();
            std::cout << "[DEBUG] Soubor: " << inputPath
                      << " obsahuje " << pageCount << " stran\n";

            // Pøidáme všechny stránky z pdfIn do pdfOut
            for (int pageIndex = 0; pageIndex < pageCount; ++pageIndex) {
                pdfOut.Append(pdfIn, pageIndex);
            }
        }

        // Výsledný dokument uložíme
        pdfOut.Write(outputFile.c_str());

        std::cout << "[INFO] Hotovo. Vystupni soubor: " << outputFile << "\n";
        return true;
    }
    catch (PdfError &err) {
        std::cerr << "[ERROR] PoDoFo selhalo pri slucovani. Code: " 
                  << err.GetError() << std::endl;
        return false;
    }
    catch (std::exception &ex) {
        std::cerr << "[ERROR] Vyjimka pri slucovani PDF: " << ex.what() << std::endl;
        return false;
    }
}

/*
 * Funkce pro konverzi obrázku (JPG, PNG) do PDF (convert) pomocí PoDoFo
 */
bool imageToPDF(const std::vector<std::string>& inputFiles, const std::string& outputFile) {
    using namespace PoDoFo;

    try {
        std::cout << "[INFO] Konvertuji obrazky (JPG/PNG) do PDF pomoci PoDoFo...\n";

        PdfMemDocument pdfOut;

        for (const auto &imgPath : inputFiles) {
            // Vytvoøíme novou stránku (A4 = 595 x 842 bodù)
            PdfRect pageRect(0.0, 0.0, 595.0, 842.0);
            PdfPage* pPage = pdfOut.CreatePage(pageRect);
            if (!pPage) {
                std::cerr << "[ERROR] Nepodarilo se vytvorit stranku pro obrazek: " 
                          << imgPath << "\n";
                continue;
            }

            PdfPainter painter;
            painter.SetPage(pPage);

            // Naèteme obrázek
            PdfImage image(&pdfOut);
            image.LoadFromFile(imgPath.c_str());

            // Rozmìry stránky
            double pageWidth  = pPage->GetPageSize().GetWidth();
            double pageHeight = pPage->GetPageSize().GetHeight();

            // Jednoduše vykreslíme obrázek pøes celou stránku
            painter.DrawImage(0.0, 0.0, &image, pageWidth, pageHeight);

            painter.FinishPage();
        }

        // Uložíme výsledný PDF do souboru
        pdfOut.Write(outputFile.c_str());

        std::cout << "[INFO] Konverze dokoncena. Vystupni soubor: " << outputFile << "\n";
        return true;
    }
    catch (PdfError &err) {
        std::cerr << "[ERROR] Chyba PoDoFo pri konverzi obrazku do PDF. Code: "
                  << err.GetError() << std::endl;
        return false;
    }
    catch (std::exception &ex) {
        std::cerr << "[ERROR] Vyjimka pri konverzi: " << ex.what() << std::endl;
        return false;
    }
}

/*
 * Funkce pro OCR (ocr) - momentálnì pseudo-kód. 
 * Pokud chcete reálnì používat Tesseract:
 *   - odkomentujte #include <tesseract/baseapi.h> a #include <leptonica/allheaders.h>
 *   - linkujte s -ltesseract -llept
 */
bool performOCR(const std::vector<std::string>& /*inputFiles*/, const std::string& /*outputFile*/) {
    std::cout << "[INFO] OCR spusteno (pseudo-kod).\n";

    // Pseudo-kód Tesseractu:
    /*
    tesseract::TessBaseAPI tess;
    if(tess.Init(nullptr, "eng")) {
        std::cerr << "Chyba: Nelze inicializovat Tesseract.\n";
        return false;
    }

    std::ofstream out(outputFile);
    if(!out.is_open()) {
        std::cerr << "Chyba: Nelze otevrit vystupni soubor.\n";
        return false;
    }

    for (const auto& f : inputFiles) {
        Pix *image = pixRead(f.c_str());
        if(!image) {
            std::cerr << "Chyba: Nelze nacist obrazek: " << f << "\n";
            continue;
        }
        tess.SetImage(image);
        char *text = tess.GetUTF8Text();
        if(text) {
            out << text << "\n";
            delete[] text;
        }
        pixDestroy(&image);
    }

    out.close();
    tess.End();
    */
    std::cout << "[INFO] OCR dokonceno (fiktivne).\n";
    return true;
}


int main(int argc, char* argv[]) {
    if (argc < 2) {
        printHelp();
        return 0;
    }

    std::vector<std::string> inputFiles;
    std::string outputFile;
    std::string functionMode;
    
    // Procházíme argumenty
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];

        if (arg == "-h" || arg == "--help") {
            printHelp();
            return 0;
        }
        else if (arg == "-v" || arg == "--version") {
            printVersion();
            return 0;
        }
        else if (arg == "-i") {
            // Všechny následující argumenty až do dalšího parametru bereme jako input soubory
            ++i;
            while (i < argc && argv[i][0] != '-') {
                inputFiles.push_back(argv[i]);
                ++i;
            }
            --i;
        }
        else if (arg == "-o") {
            if ((i + 1) < argc) {
                outputFile = argv[++i];
            } else {
                std::cerr << "Chyba: Nebyl zadan vystupni soubor.\n";
                return 1;
            }
        }
        else if (arg == "-f") {
            if ((i + 1) < argc) {
                functionMode = argv[++i];
            } else {
                std::cerr << "Chyba: Nebyla zadana funkce (blend/convert/ocr).\n";
                return 1;
            }
        }
        else {
            std::cerr << "Neznamy parametr: " << arg << "\n";
            return 1;
        }
    }

    if (functionMode.empty()) {
        std::cerr << "Chyba: Nebyla zadana funkce. Pouzijte -f (blend|convert|ocr).\n";
        return 1;
    }
    if (inputFiles.empty()) {
        std::cerr << "Chyba: Nebyly zadany vstupni soubory. Pouzijte -i.\n";
        return 1;
    }

    // Dle zvolené funkce
    if (functionMode == "blend") {
        if (outputFile.empty()) {
            std::cerr << "Chyba: Nebyl zadan vystupni soubor pro slouceni PDF.\n";
            return 1;
        }
        if (!mergePDFs(inputFiles, outputFile)) {
            std::cerr << "Chyba: Slouceni PDF selhalo.\n";
            return 1;
        }
    }
    else if (functionMode == "convert") {
        if (outputFile.empty()) {
            std::cerr << "Chyba: Nebyl zadan vystupni soubor pro konverzi.\n";
            return 1;
        }
        if (!imageToPDF(inputFiles, outputFile)) {
            std::cerr << "Chyba: Konverze do PDF selhala.\n";
            return 1;
        }
    }
    else if (functionMode == "ocr") {
        if (outputFile.empty()) {
            std::cerr << "Chyba: Nebyl zadan vystupni .txt soubor.\n";
            return 1;
        }
        if (!performOCR(inputFiles, outputFile)) {
            std::cerr << "Chyba: OCR selhalo.\n";
            return 1;
        }
    }
    else {
        std::cerr << "Chyba: Neznama funkce: " << functionMode << "\n";
        return 1;
    }

    return 0;
}
