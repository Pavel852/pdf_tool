/*
 * pdf_tool
 * Autor: PB (pavel.bartos.pb@gmail.com)
 * Licence: CC
 *
 * Implementace funkcí:
 * 1) Slouèení PDF (blend)  -- pomocí PoDoFo
 * 2) Konverze JPG/PNG do PDF (convert) -- pomocí PoDoFo
 * 3) OCR (jpg/png/pdf do .txt) -- pseudo-kód pro Tesseract
 */

#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <ctime>       // pro pøípadné generování unikátního jména, pokud byste chtìli

// -------------- PoDoFo --------------
#include <podofo/podofo.h>

// -------------- Tesseract OCR (pseudo) --------------
// #include <tesseract/baseapi.h>
// #include <leptonica/allheaders.h>

// Pomocná funkce pro zobrazení nápovìdy (help)
void printHelp() {
    std::cout 
        << "Pouziti: pdf_tool [parametry]\n"
        << "Parametry:\n"
        << "  -h, --help       Zobrazi tuto napovedu.\n"
        << "  -v, --version    Zobrazi verzi a informace o autorovi.\n"
        << "  -i <soubory>     Vstupni soubory (PDF, JPG, PNG). Lze zadat vice.\n"
        << "  -o <soubor>      Vystupni soubor (neni povinny). Pokud neni zadano,\n"
        << "                   pouzije se automaticky 'output.pdf' nebo 'output.txt'.\n"
        << "  -f <funkce>      Funkce:\n"
        << "                   blend   - Slouceni PDF do jednoho\n"
        << "                   convert - Konverze JPG/PNG do PDF\n"
        << "                   ocr     - OCR (z PDF/JPG/PNG do txt)\n"
        << "\nPriklad:\n"
        << "  pdf_tool -f blend -i file1.pdf file2.pdf file3.pdf\n"
        << "  pdf_tool -f convert -i obrazek.jpg pic.png\n"
        << "  pdf_tool -f ocr -i scan.pdf pic.png\n";
}

// Pomocná funkce pro zobrazení verze
void printVersion() {
    std::cout 
        << "pdf_tool v1.0\n"
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

        for (const auto &inputPath : inputFiles) {
            // Ovìøit, zda soubor jde otevøít
            std::ifstream ifs(inputPath, std::ios::binary);
            if (!ifs.good()) {
                std::cerr << "[ERROR] Soubor '" << inputPath 
                          << "' nelze otevrit. Neexistuje nebo chybi prava.\n";
                return false;
            }
            ifs.close();

            PdfMemDocument pdfIn;
            pdfIn.Load(inputPath.c_str());  // Mùže vyhodit výjimku (napø. kód 8)
            int pageCount = pdfIn.GetPageCount();
            std::cout << "[DEBUG] Soubor: " << inputPath
                      << " obsahuje " << pageCount << " stran\n";

            for (int pageIndex = 0; pageIndex < pageCount; ++pageIndex) {
                pdfOut.Append(pdfIn, pageIndex);
            }
        }

        pdfOut.Write(outputFile.c_str());
        std::cout << "[INFO] Hotovo. Výstupní soubor: " << outputFile << "\n";
        return true;
    }
    catch (PdfError &err) {
        std::cerr << "[ERROR] PoDoFo selhalo pri slucovani. Code: " 
                  << err.GetError() << "\n";
        return false;
    }
    catch (std::exception &ex) {
        std::cerr << "[ERROR] Vyjimka pri slucovani PDF: " << ex.what() << "\n";
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
            // Ovìøit, zda soubor jde otevøít
            std::ifstream ifs(imgPath, std::ios::binary);
            if (!ifs.good()) {
                std::cerr << "[ERROR] Soubor '" << imgPath 
                          << "' nelze otevrit.\n";
                continue; // nebo return false, dle potøeby
            }
            ifs.close();

            // Vytvoøíme novou stránku (A4 = 595 x 842 bodù)
            PdfRect pageRect(0.0, 0.0, 595.0, 842.0);
            PdfPage* pPage = pdfOut.CreatePage(pageRect);
            if (!pPage) {
                std::cerr << "[ERROR] Nepodarilo se vytvorit stranku pro: " 
                          << imgPath << "\n";
                continue;
            }

            PdfPainter painter;
            painter.SetPage(pPage);

            PdfImage image(&pdfOut);
            image.LoadFromFile(imgPath.c_str());

            double pageWidth  = pPage->GetPageSize().GetWidth();
            double pageHeight = pPage->GetPageSize().GetHeight();

            painter.DrawImage(0.0, 0.0, &image, pageWidth, pageHeight);
            painter.FinishPage();
        }

        pdfOut.Write(outputFile.c_str());
        std::cout << "[INFO] Konverze dokoncena. Vystupni soubor: " << outputFile << "\n";
        return true;
    }
    catch (PdfError &err) {
        std::cerr << "[ERROR] Chyba PoDoFo pri konverzi obrazku do PDF. Code: "
                  << err.GetError() << "\n";
        return false;
    }
    catch (std::exception &ex) {
        std::cerr << "[ERROR] Vyjimka pri konverzi: " << ex.what() << "\n";
        return false;
    }
}

/*
 * Funkce pro OCR (ocr) - momentálnì pseudo-kód.
 * Pokud chcete reálnì používat Tesseract, musíte sem doplnit volání:
 *   tesseract::TessBaseAPI, pixRead(), atd.
 */
bool performOCR(const std::vector<std::string>& inputFiles, const std::string& outputFile) {
    std::cout << "[INFO] OCR spusteno (pseudo-kod). Vystup: " << outputFile << "\n";

    // Zatím to jen simuluje.
    // V reálné implementaci se Tesseractem byste:
    // 1) Otevøeli Tesseract
    // 2) Pro každý soubor (PDF, JPG, PNG) -> konverze do bitové mapy (pokud PDF, tak render)
    // 3) Získali text a uložili do outputFile

    std::ofstream out(outputFile);
    if (!out.is_open()) {
        std::cerr << "[ERROR] Nelze otevrit vystupni soubor: " << outputFile << "\n";
        return false;
    }

    // Pøíklad: zapíšeme jen informaci, co by se dìlalo reálnì
    out << "Fiktivni OCR vysledky...\n";
    for (const auto& f : inputFiles) {
        out << "[OCR] Zpracovan soubor: " << f << "\n";
    }

    out.close();
    std::cout << "[INFO] OCR dokonceno (fiktivne).\n";
    return true;
}

// ------------------------------------------------

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
            // Všechny následující argumenty až do dalšího parametru
            // bereme jako input soubory
            ++i;
            while (i < argc && argv[i][0] != '-') {
                inputFiles.push_back(argv[i]);
                ++i;
            }
            // Protože for-cyklus sám i++ provede,
            // tak vrátíme index o 1 zpìt
            --i;
        }
        else if (arg == "-o") {
            // Následující argument je výstupní soubor
            if ((i + 1) < argc) {
                outputFile = argv[++i];
            } else {
                std::cerr << "Chyba: Nebyl zadan vystupni soubor.\n";
                return 1;
            }
        }
        else if (arg == "-f") {
            // Následující argument je funkce
            if ((i + 1) < argc) {
                functionMode = argv[++i];
            } else {
                std::cerr << "Chyba: Nebyla zadana funkce (blend/convert/ocr).\n";
                return 1;
            }
        }
        else {
            // Neznámý parametr
            std::cerr << "Neznamy parametr: " << arg << "\n";
            return 1;
        }
    }

    // Ošetøení, zda máme zadané vstupní soubory a funkci
    if (functionMode.empty()) {
        std::cerr << "Chyba: Nebyla zadana funkce. Pouzijte -f (blend|convert|ocr).\n";
        return 1;
    }
    if (inputFiles.empty()) {
        std::cerr << "Chyba: Nebyly zadany vstupni soubory. Pouzijte -i.\n";
        return 1;
    }

    // Pokud uzivatel nezadal -o, vygenerujeme vychozi
    if (outputFile.empty()) {
        if (functionMode == "ocr") {
            outputFile = "output.txt";
        } else {
            // pro blend a convert
            outputFile = "output.pdf";
        }
        std::cout << "[INFO] Nebyl zadan -o, pouzivam vychozi: " << outputFile << "\n";
    }

    // Dle zvolené funkce se spustí pøíslušná akce
    if (functionMode == "blend") {
        if (!mergePDFs(inputFiles, outputFile)) {
            std::cerr << "Chyba: Slouceni PDF selhalo.\n";
            return 1;
        }
    }
    else if (functionMode == "convert") {
        if (!imageToPDF(inputFiles, outputFile)) {
            std::cerr << "Chyba: Konverze do PDF selhala.\n";
            return 1;
        }
    }
    else if (functionMode == "ocr") {
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
