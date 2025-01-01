/*
 * pdf_tool
 * Autor: PB (pavel.bartos.pb@gmail.com)
 * Licence: CC
 *
 * Implementace (bez PoDoFo):
 * 1) Slou�en� PDF soubor� (blend) pomoc� extern�ho n�stroje QPDF
 * 2) Konverze obr�zk� (JPG/PNG) do PDF (convert) pomoc� extern�ho n�stroje ImageMagick
 * 3) OCR (jpg/png/pdf do .txt) - pseudo-k�d (mo�no nahradit vol�n�m Tesseract)
 */

#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>      // pro sestavov�n� p��kaz�
#include <cstdlib>      // system()

// Pomocn� funkce pro zobrazen� n�pov�dy (help)
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

// Pomocn� funkce pro zobrazen� verze
void printVersion() {
    std::cout 
        << "pdf_tool v1.8 (bez PoDoFo)\n"
        << "Autor: PB, pavel.bartos.pb@gmail.com\n"
        << "License: CC\n";
}

/*
 * Funkce pro slou�en� PDF (blend)
 * Vyu��v�me extern� n�stroj QPDF:
 *   qpdf --empty --pages file1.pdf file2.pdf ... -- out.pdf
 */
bool mergePDFs(const std::vector<std::string>& inputFiles, const std::string& outputFile) {
    // Z�kladn� ov��en� existence a �itelnosti soubor�
    for (const auto &f : inputFiles) {
        std::ifstream test(f);
        if (!test.good()) {
            std::cerr << "[ERROR] Soubor '" << f << "' neexistuje nebo jej nelze otevrit.\n";
            return false;
        }
    }

    // Sestav�me p��kaz pro QPDF
    // qpdf --empty --pages file1.pdf file2.pdf ... -- out.pdf
    std::ostringstream cmd;
    cmd << "qpdf --empty --pages";
    for (const auto &f : inputFiles) {
        // Pozor: Pokud by n�zvy soubor� obsahovaly mezery, je t�eba je escapovat
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
 * Funkce pro konverzi obr�zku (JPG, PNG) do PDF (convert)
 * Vyu��v�me extern� n�stroj ImageMagick (nap�. "convert" nebo "magick convert"):
 *
 *   magick convert img1.jpg img2.png out.pdf
 * (p��padn� star�� verze: convert img1.jpg ...)
 */
bool imageToPDF(const std::vector<std::string>& inputFiles, const std::string& outputFile) {
    // Ov��en� existence vstupn�ch soubor�
    for (const auto &f : inputFiles) {
        std::ifstream test(f, std::ios::binary);
        if (!test.good()) {
            std::cerr << "[ERROR] Obrazek '" << f << "' nelze otevrit.\n";
            return false;
        }
    }

    // Sestav�me p��kaz pro ImageMagick
    // magick convert file1.jpg file2.png ... output.pdf
    // (N�kdy sta�� "convert" m�sto "magick convert", podle verze syst�mu)
    std::ostringstream cmd;
    cmd << "magick convert";  // pokud m�te star�� verzi, m��e to b�t jen "convert"
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
 * Funkce pro OCR (ocr) - pseudo-k�d
 * V praxi byste mohli spustit Tesseract externim volanim, napr.:
 *   tesseract input.png output.txt
 * Nebo pou��t Tesseract C++ API (baseapi.h).
 */
bool performOCR(const std::vector<std::string>& inputFiles, const std::string& outputFile) {
    std::cout << "[INFO] OCR spusteno (pseudo-kod). Vystup: " << outputFile << "\n";

    // Demonstrativn� zap�eme jen "fiktivn�" v�stup
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
    
    // Proch�z�me argumenty
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
            // V�echny n�sleduj�c� argumenty a� do dal��ho parametru
            // bereme jako input soubory
            ++i;
            while (i < argc && argv[i][0] != '-') {
                inputFiles.push_back(argv[i]);
                ++i;
            }
            --i; // Vr�t�me se o 1
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
            // Nezn�m� parametr
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

    // Dle zvolen� funkce se spust� p��slu�n� akce
    if (functionMode == "blend") {
        // Slou�en� PDF pomoc� QPDF
        if (!mergePDFs(inputFiles, outputFile)) {
            std::cerr << "Chyba: Slouceni PDF selhalo.\n";
            return 1;
        }
    }
    else if (functionMode == "convert") {
        // Konverze obr�zk� do PDF (ImageMagick)
        if (!imageToPDF(inputFiles, outputFile)) {
            std::cerr << "Chyba: Konverze do PDF selhala.\n";
            return 1;
        }
    }
    else if (functionMode == "ocr") {
        // Pseudo-k�d OCR (vol�n� Tesseractu by se muselo dod�lat)
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
