/*
 * pdf_tool
 * Autor: PB (pavel.bartos.pb@gmail.com)
 * Licence: CC
 *
 * Implementace (bez PoDoFo):
 * 1) Slouèení PDF souborù (blend) pomocí externího nástroje QPDF
 * 2) Konverze obrázkù (JPG/PNG) do PDF (convert) pomocí externího nástroje ImageMagick
 * 3) OCR (jpg/png/pdf do .txt) - pseudo-kód (možno nahradit voláním Tesseract)
 */

#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>      // pro sestavování pøíkazù
#include <cstdlib>      // system()

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
        << "                   blend   - Slouceni PDF do jednoho (QPDF)\n"
        << "                   convert - Konverze JPG/PNG do PDF (ImageMagick)\n"
        << "                   ocr     - OCR (z PDF/JPG/PNG do txt) [pseudo-kod]\n"
        << "\nPriklad:\n"
        << "  pdf_tool -f blend   -i file1.pdf file2.pdf -o merged.pdf\n"
        << "  pdf_tool -f convert -i obrazek.jpg pic.png -o vystup.pdf\n"
        << "  pdf_tool -f ocr     -i scan.pdf pic.png -o vysledek.txt\n";
}

// Pomocná funkce pro zobrazení verze
void printVersion() {
    std::cout 
        << "pdf_tool v1.8 (bez PoDoFo)\n"
        << "Autor: PB, pavel.bartos.pb@gmail.com\n"
        << "License: CC\n";
}

/*
 * Funkce pro slouèení PDF (blend)
 * Využíváme externí nástroj QPDF:
 *   qpdf --empty --pages file1.pdf file2.pdf ... -- out.pdf
 */
bool mergePDFs(const std::vector<std::string>& inputFiles, const std::string& outputFile) {
    // Základní ovìøení existence a èitelnosti souborù
    for (const auto &f : inputFiles) {
        std::ifstream test(f);
        if (!test.good()) {
            std::cerr << "[ERROR] Soubor '" << f << "' neexistuje nebo jej nelze otevrit.\n";
            return false;
        }
    }

    // Sestavíme pøíkaz pro QPDF
    // qpdf --empty --pages file1.pdf file2.pdf ... -- out.pdf
    std::ostringstream cmd;
    cmd << "qpdf --empty --pages";
    for (const auto &f : inputFiles) {
        // Pozor: Pokud by názvy souborù obsahovaly mezery, je tøeba je escapovat
        cmd << " " << f;
    }
    cmd << " -- " << outputFile;

    std::cout << "[INFO] Spoustim QPDF pro slouceni PDF:\n" << cmd.str() << "\n";

    int ret = std::system(cmd.str().c_str());
    if (ret != 0) {
        std::cerr << "[ERROR] qpdf selhalo (navratovy kod: " << ret << ").\n";
        return false;
    }

    std::cout << "[INFO] Soubory byly uspesne slouceny do: " << outputFile << "\n";
    return true;
}

/*
 * Funkce pro konverzi obrázku (JPG, PNG) do PDF (convert)
 * Využíváme externí nástroj ImageMagick (napø. "convert" nebo "magick convert"):
 *
 *   magick convert img1.jpg img2.png out.pdf
 * (pøípadnì starší verze: convert img1.jpg ...)
 */
bool imageToPDF(const std::vector<std::string>& inputFiles, const std::string& outputFile) {
    // Ovìøení existence vstupních souborù
    for (const auto &f : inputFiles) {
        std::ifstream test(f, std::ios::binary);
        if (!test.good()) {
            std::cerr << "[ERROR] Obrazek '" << f << "' nelze otevrit.\n";
            return false;
        }
    }

    // Sestavíme pøíkaz pro ImageMagick
    // magick convert file1.jpg file2.png ... output.pdf
    // (Nìkdy staèí "convert" místo "magick convert", podle verze systému)
    std::ostringstream cmd;
    cmd << "magick convert";  // pokud máte starší verzi, mùže to být jen "convert"
    for (const auto &f : inputFiles) {
        cmd << " " << f;
    }
    cmd << " " << outputFile;

    std::cout << "[INFO] Spoustim ImageMagick pro konverzi obrazku:\n" 
              << cmd.str() << "\n";

    int ret = std::system(cmd.str().c_str());
    if (ret != 0) {
        std::cerr << "[ERROR] ImageMagick (convert) selhalo (kod: " 
                  << ret << ").\n";
        return false;
    }

    std::cout << "[INFO] Obrazky byly uspesne konvertovany do: " << outputFile << "\n";
    return true;
}

/*
 * Funkce pro OCR (ocr) - pseudo-kód
 * V praxi byste mohli spustit Tesseract externim volanim, napr.:
 *   tesseract input.png output.txt
 * Nebo použít Tesseract C++ API (baseapi.h).
 */
bool performOCR(const std::vector<std::string>& inputFiles, const std::string& outputFile) {
    std::cout << "[INFO] OCR spusteno (pseudo-kod). Vystup: " << outputFile << "\n";

    // Demonstrativnì zapíšeme jen "fiktivní" výstup
    std::ofstream ofs(outputFile);
    if (!ofs.is_open()) {
        std::cerr << "[ERROR] Nelze otevrit vystupni soubor: " << outputFile << "\n";
        return false;
    }

    ofs << "Fiktivni OCR vysledky (pouzijte realnou implementaci napr. Tesseract)...\n\n";
    for (const auto& f : inputFiles) {
        ofs << "[OCR] Zpracovan soubor: " << f << "\n";
    }

    ofs.close();
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
            --i; // Vrátíme se o 1
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
            // Neznámý parametr
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

    // Pokud uzivatel nezadal -o, vybereme vychozi
    if (outputFile.empty()) {
        if (functionMode == "ocr") {
            outputFile = "output.txt";
        } else {
            outputFile = "output.pdf";
        }
        std::cout << "[INFO] Nebyl zadan -o, pouzivam vychozi: " 
                  << outputFile << "\n";
    }

    // Dle zvolené funkce se spustí pøíslušná akce
    if (functionMode == "blend") {
        // Slouèení PDF pomocí QPDF
        if (!mergePDFs(inputFiles, outputFile)) {
            std::cerr << "Chyba: Slouceni PDF selhalo.\n";
            return 1;
        }
    }
    else if (functionMode == "convert") {
        // Konverze obrázkù do PDF (ImageMagick)
        if (!imageToPDF(inputFiles, outputFile)) {
            std::cerr << "Chyba: Konverze do PDF selhala.\n";
            return 1;
        }
    }
    else if (functionMode == "ocr") {
        // Pseudo-kód OCR (volání Tesseractu by se muselo dodìlat)
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
