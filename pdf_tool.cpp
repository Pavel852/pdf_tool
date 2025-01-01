/*
 * pdf_tool
 * Autor: PB (pavel.bartos.pb@gmail.com)
 * Licence: CC
 *
 * Implementace funkc�:
 * 1) Slou�en� PDF soubor� do jednoho (blend)  -- pomoc� PoDoFo
 * 2) Konverze JPG/PNG do PDF (convert)        -- pomoc� PoDoFo
 * 3) OCR (jpg/png/pdf do .txt)                -- uk�zkov�/pseudo-k�d pro Tesseract
 *
 */

#include <iostream>
#include <string>
#include <vector>
#include <fstream>

// -------------- PoDoFo --------------
#include <podofo/podofo.h>
#include <podofo/pdfpage.h>
// P�i pou�it� PoDoFo je �asto t�eba linkovat s -lpodofo a pou��t pkg-config

// -------------- Tesseract OCR (pseudo / pro realnou implementaci odkomentovat) --------------
// #include <tesseract/baseapi.h>
// #include <leptonica/allheaders.h>


// Pomocn� funkce pro zobrazen� n�pov�dy (help)
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

// Pomocn� funkce pro zobrazen� verze
void printVersion() {
    std::cout << "pdf_tool v1.4\n"
              << "Autor: PB, pavel.bartos.pb@gmail.com\n"
              << "License: CC\n";
}

/*
 * Funkce pro slou�en� PDF (blend) pomoc� knihovny PoDoFo
 */
bool mergePDFs(const std::vector<std::string>& inputFiles, const std::string& outputFile) {
    try {
        std::cout << "[INFO] Slu�uji PDF pomoc� PoDoFo...\n";

        PoDoFo::PdfMemDocument pdfOut;

        // Pro ka�d� vstupn� soubor
        for (const auto &inputPath : inputFiles) {
            // Na�teme PDF do do�asn�ho dokumentu
            PoDoFo::PdfMemDocument pdfIn;
            pdfIn.Load(inputPath.c_str());

            // Zjist�me po�et str�nek
            int pageCount = pdfIn.GetPageCount();
            std::cout << "[DEBUG] Soubor: " << inputPath
                      << " obsahuje " << pageCount << " stran\n";

            // P�id�me v�echny str�nky z pdfIn do pdfOut
            for (int pageIndex = 0; pageIndex < pageCount; ++pageIndex) {
                pdfOut.Append(pdfIn, pageIndex);
            }
        }

        // V�sledn� dokument ulo��me
        pdfOut.Write(outputFile.c_str());

        std::cout << "[INFO] Hotovo. Vystupni soubor: " << outputFile << "\n";
        return true;
    }
    catch (PoDoFo::PdfError &err) {
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
 * Funkce pro konverzi obr�zku (JPG, PNG) do PDF (convert) pomoc� PoDoFo
 */
bool imageToPDF(const std::vector<std::string>& inputFiles, const std::string& outputFile) {
    using namespace PoDoFo;

    try {
        std::cout << "[INFO] Konvertuji obrazky (JPG/PNG) do PDF pomoci PoDoFo...\n";

        PdfMemDocument pdfOut;

        // Vytvo��me str�nku pro ka�d� obr�zek (nebo m��ete slou�it v�e na jednu str�nku)
        for (const auto &imgPath : inputFiles) {
            // Vytvo��me novou str�nku. Nastav�me velikost na A4 (���ka 595, v��ka 842) 
            // nebo m��eme pou��t re�ln� rozm�ry obr�zku (pomoc� image.GetWidth() atd.).
            PdfPage* pPage = pdfOut.CreatePage(PdfPageFormat::A4());
            if (!pPage) {
                std::cerr << "[ERROR] Nepodarilo se vytvorit stranku pro obrazek: " << imgPath << "\n";
                continue;
            }

            // Inicializace "mal��e" (painter)
            PdfPainter painter;
            painter.SetPage(pPage);

            // Na�teme obr�zek
            PdfImage image(&pdfOut);
            image.LoadFromFile(imgPath.c_str());

            double pageWidth  = pPage->GetPageSize().GetWidth();
            double pageHeight = pPage->GetPageSize().GetHeight();

            // Zjist�me rozm�ry obr�zku v bodech (72 DPI). Pokud chceme zachovat pom�r stran, 
            // m��eme nap��klad dopo��tat vhodnou ���ku/v��ku:
            double imgWidth  = image.GetWidth();
            double imgHeight = image.GetHeight();

            // Pro jednoduchost: nech�me obr�zek vyplnit celou str�nku
            painter.DrawImage(0, 0, &image, pageWidth, pageHeight);

            painter.FinishPage();
        }

        // Ulo��me v�sledn� PDF do souboru
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
 * Funkce pro OCR (ocr) - Uk�zkov� pseudo-k�d s Tesseractem
 * V re�ln� implementaci je nutn� m�t Tesseract a Leptonicu, 
 * a volat funkce pro zpracov�n�. Zde je ponech�n sp�e demonstrativn� skeleton.
 */
bool performOCR(const std::vector<std::string>& inputFiles, const std::string& outputFile) {
    std::cout << "[INFO] OCR spusteno (pseudo-kod). Vystup: " << outputFile << "\n";

    // Pseudo-k�d:
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
    // Zde by se tak� �e�ilo, pokud je vstup PDF (bez textov� vrstvy),
    // tak byste museli PDF nejprve rendrovat do obr�zku a ten pak poslat do Tesseractu.

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
            // V�echny n�sleduj�c� argumenty a� do dal��ho parametru bereme jako input soubory
            ++i;
            while (i < argc && argv[i][0] != '-') {
                inputFiles.push_back(argv[i]);
                ++i;
            }
            --i; // Vr�t�me se o jeden zp�t, proto�e v for-cyklu se je�t� zv���
        }
        else if (arg == "-o") {
            // N�sleduj�c� argument je v�stupn� soubor
            if ((i + 1) < argc) {
                outputFile = argv[++i];
            } else {
                std::cerr << "Chyba: Nebyl zadan vystupni soubor.\n";
                return 1;
            }
        }
        else if (arg == "-f") {
            // N�sleduj�c� argument je funkce
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

    // O�et�en�, zda m�me zadan� vstupn� soubory a funkci
    if (functionMode.empty()) {
        std::cerr << "Chyba: Nebyla zadana funkce. Pouzijte -f (blend|convert|ocr).\n";
        return 1;
    }
    if (inputFiles.empty()) {
        std::cerr << "Chyba: Nebyly zadany vstupni soubory. Pouzijte -i.\n";
        return 1;
    }

    // Dle zvolen� funkce se spust� p��slu�n� akce
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
