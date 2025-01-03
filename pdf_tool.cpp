/*
 * pdf_tool.cpp
 *
 * Version: 1.0
 * Author:  PB, pavel.bartos.pb@gmail.com
 * License: CC
 *
 * Nástroj demonstrující sloučení PDF/obrázků do jednoho PDF (pomocí QPDF)
 * a OCR z PDF/obrázků do textu (pomocí Tesseract).
 */

#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <cstdio>

// QPDF
#include <qpdf/QPDF.hh>
#include <qpdf/QPDFWriter.hh>
#include <qpdf/QPDFPageDocumentHelper.hh>
#include <qpdf/QPDFObjectHandle.hh>

// Tesseract
#include <tesseract/baseapi.h>
#include <leptonica/allheaders.h>

/**
 * Konverze obrázku (JPG/PNG) na PDF.
 *
 * V reálné implementaci byste využili:
 *  - buď funkci z externí knihovny,
 *  - nebo zavolali např. ImageMagick (convert) / Ghostscript a uložili do dočasného PDF.
 *
 * Zde je to jen *schematická* funkce vracející jméno "temp_img_XXX.pdf"
 */
std::string createPDFfromImage(const std::string& imagePath) {
    // Zde by byla reálná konverze (např. volání "convert <imagePath> -> docasny.pdf")
    static int counter = 0; 
    std::string tempPDF = "temp_img_" + std::to_string(counter++) + ".pdf";

    // Pro ukázku jen vypíšeme hlášku a "předstíráme", že konverze proběhla:
    std::cout << "[INFO] (simulate) Konverze obrazku na PDF: " << imagePath 
              << " -> " << tempPDF << std::endl;

    return tempPDF;
}

/**
 * Sloučení zadaných PDF/PNG/JPG souborů do jednoho PDF (pomocí QPDF).
 *
 * 1) Vytvoříme prázdný QPDF dokument (outpdf.emptyPDF()).
 * 2) Pro každý vstupní soubor:
 *    - Pokud je to PDF, rovnou jej načteme QPDFem.
 *    - Pokud je to obrázek (JPG/PNG), nejprve zavoláme createPDFfromImage(),
 *      a pak načteme dočasný PDF.
 * 3) Z každého PDF extrahujeme stránky a přidáme je do hlavního PDF dokumentu.
 * 4) Uložíme hotový výsledek do `outputFile`.
 */
bool mergeFilesIntoPDF(const std::vector<std::string>& inputFiles, const std::string& outputFile) {
    QPDF outpdf;
    // Vytvoříme prázdný PDF dokument
    outpdf.emptyPDF();

    for (auto &file : inputFiles) {
        // Kontrola přípony
        auto toLower = [](std::string s) {
            std::transform(s.begin(), s.end(), s.begin(), ::tolower);
            return s;
        };
        std::string lowerFile = toLower(file);

        std::string pdfToImport = file;
        // Pokud není .pdf, považujeme za obrázek
        if (lowerFile.rfind(".pdf") == std::string::npos) {
            pdfToImport = createPDFfromImage(file);
            if (pdfToImport.empty()) {
                std::cerr << "[ERROR] Chyba pri konverzi " << file 
                          << " na PDF (fce createPDFfromImage selhala)." << std::endl;
                return false;
            }
        }

        // Nyní načteme PDF pomocí QPDF
        QPDF tmpPdf;
        try {
            tmpPdf.processFile(pdfToImport.c_str());
        } catch (std::exception &e) {
            std::cerr << "[ERROR] Nelze nacist PDF " << pdfToImport 
                      << ": " << e.what() << std::endl;
            return false;
        }

        // Vytáhneme stránky ze zdrojového PDF
        QPDFPageDocumentHelper outpdfHelper(outpdf);
        QPDFPageDocumentHelper tmpHelper(tmpPdf);

        // Získáme všechny stránky zdrojového PDF
        auto pages = tmpHelper.getAllPages();
        // Místo addPages() (které ve starší verzi QPDF nemusí existovat)
        // použijeme v cyklu addPage() pro každou stránku
        for (auto const& page : pages) {
            outpdfHelper.addPage(page, false);
        }
    }

    // Uložení výsledného PDF
    try {
        QPDFWriter writer(outpdf, outputFile.c_str());
        writer.setStaticID(true);       // nechceme generovat náhodný ID
        writer.setLinearization(false); // nepotřebujeme linearizaci
        writer.write();
    } catch (std::exception &e) {
        std::cerr << "[ERROR] Chyba pri ukladani PDF: " << outputFile 
                  << " - " << e.what() << std::endl;
        return false;
    }

    return true;
}

/**
 * Provede OCR přes Tesseract pro dané PDF/PNG/JPG soubory a uloží do jednoho textového souboru.
 *
 * Pozn.: QPDF neřeší vykreslování PDF do bitmap (to by musel obsloužit jiný nástroj/knihovna,
 * např. Poppler). Zde *schematicky* převezmeme předchozí ukázku, 
 * tzn. pro PDF "předpokládáme" nějaké vykreslení do bitmapy apod.
 */
bool performOCR(const std::vector<std::string>& inputFiles, const std::string& outputFile) {
    FILE* out = fopen(outputFile.c_str(), "w");
    if (!out) {
        std::cerr << "[ERROR] Nelze otevrit vystupni soubor pro zapis: " << outputFile << std::endl;
        return false;
    }

    // Inicializace Tesseractu
    tesseract::TessBaseAPI ocr;
    if (ocr.Init(nullptr, "eng")) {
        std::cerr << "[ERROR] Nelze inicializovat Tesseract (eng)" << std::endl;
        fclose(out);
        return false;
    }

    for (auto &file : inputFiles) {
        auto toLower = [](std::string s) {
            std::transform(s.begin(), s.end(), s.begin(), ::tolower);
            return s;
        };
        std::string lowerFile = toLower(file);

        // Rozlišení PDF vs obrázek
        // (Pokud je PDF, tak zjednodušeně simuluje vykreslení - v praxi byste použili např. Poppler pro rasterizaci.)
        if (lowerFile.rfind(".pdf") != std::string::npos) {
            std::cout << "[WARN] OCR PDF: Tato ukazka nerozumi praci s PDF pro OCR. "
                      << "V realu byste museli pouzit rasterizaci (Poppler, Cairo, ...)."
                      << std::endl;
            fprintf(out, "===== [File: %s] (PDF OCR not fully implemented) =====\n", file.c_str());
            // Sem by přišla reálná implementace rasterizace PDF -> bitmap, poté Tesseract -> text.
        } 
        else {
            // Je to obrázek
            Pix* image = pixRead(file.c_str());
            if (!image) {
                std::cerr << "[ERROR] Nelze nacist obrazek pro OCR: " << file << std::endl;
                continue;
            }
            ocr.SetImage(image);
            char* ocrResult = ocr.GetUTF8Text();
            if (ocrResult) {
                fprintf(out, "===== [File: %s] =====\n", file.c_str());
                fprintf(out, "%s\n", ocrResult);
                delete[] ocrResult;
            }
            pixDestroy(&image);
        }
    }

    fclose(out);
    ocr.End();
    return true;
}

// Vypsat nápovědu
void printHelp() {
    std::cout << "pdf_tool - A simple CLI for PDF and image manipulation (QPDF + Tesseract).\n"
              << "Usage:\n"
              << "  pdf_tool [options]\n\n"
              << "Options:\n"
              << "  -h, --help        Zobrazi napovedu.\n"
              << "  -v, --version     Zobrazi verzi programu a informace o autorovi.\n"
              << "  -i <soubory>      Seznam souboru (PDF, JPG, PNG) k vstupu.\n"
              << "  -o <soubor>       Cesta k vystupnimu souboru.\n"
              << "  -f=blend          Slouci zvolene PDF/JPG/PNG soubory do jednoho PDF.\n"
              << "  -f=ocr            Provede OCR (z PDF/JPG/PNG) do .txt vystupu.\n"
              << std::endl;
}

// Vypsat verzi
void printVersion() {
    std::cout << "pdf_tool\n"
              << "Version: 1.0\n"
              << "Author:  PB, pavel.bartos.pb@gmail.com\n"
              << "License: CC\n"
              << std::endl;
}

// Pomocná funkce pro získání hodnoty za --param nebo -param
std::string getArgValue(int& i, char** argv, int argc) {
    if (i + 1 < argc) {
        return std::string(argv[++i]);
    } else {
        std::cerr << "[ERROR] Missing value for argument: " << argv[i] << std::endl;
        exit(1);
    }
}

int main(int argc, char** argv)
{
    if (argc < 2) {
        // Bez parametru - vypíšeme nápovědu
        printHelp();
        return 0;
    }

    // Proměnné pro argumenty
    bool doHelp = false;
    bool doVersion = false;
    std::string functionMode;     // "blend" nebo "ocr"
    std::vector<std::string> inputFiles;
    std::string outputFile;       // pokud není zadané, bude "output.pdf" nebo "output.txt"

    // Zpracování argumentů
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];

        // --help nebo -h
        if (arg == "-h" || arg == "--help") {
            doHelp = true;
        }
        // --version nebo -v
        else if (arg == "-v" || arg == "--version") {
            doVersion = true;
        }
        // -i <soubory>
        else if (arg == "-i") {
            while ((i + 1 < argc) && (argv[i + 1][0] != '-')) {
                inputFiles.push_back(std::string(argv[++i]));
            }
        }
        // -o <soubor> (vystup)
        else if (arg == "-o") {
            outputFile = getArgValue(i, argv, argc);
        }
        // -f=blend nebo -f=ocr
        else if (arg.rfind("-f=", 0) == 0) {
            // oddělíme hodnotu za '='
            functionMode = arg.substr(3);
        }
        else {
            std::cerr << "[WARN] Ignored unknown argument: " << arg << std::endl;
        }
    }

    // Ošetření -h / --help
    if (doHelp) {
        printHelp();
        return 0;
    }

    // Ošetření -v / --version
    if (doVersion) {
        printVersion();
        return 0;
    }

    // Kontrola, zda máme funkci k vykonání
    if (functionMode.empty()) {
        std::cerr << "[ERROR] Nebyl zadan parametr -f=blend|ocr" << std::endl;
        return 1;
    }

    // Kontrola, zda máme nějaké input soubory
    if (inputFiles.empty()) {
        std::cerr << "[ERROR] Nebyly zadany vstupni soubory pomoci -i" << std::endl;
        return 1;
    }

    // Pokud uživatel nezadal -o, nastavíme implicitně.
    if (outputFile.empty()) {
        if (functionMode == "blend") {
            outputFile = "output.pdf";
        } else if (functionMode == "ocr") {
            outputFile = "output.txt";
        }
    }

    // Rozhodnutí podle funkce
    bool result = false;
    if (functionMode == "blend") {
        result = mergeFilesIntoPDF(inputFiles, outputFile);
        if (!result) {
            std::cerr << "[ERROR] Slouceni PDF selhalo." << std::endl;
        }
    }
    else if (functionMode == "ocr") {
        result = performOCR(inputFiles, outputFile);
        if (!result) {
            std::cerr << "[ERROR] OCR selhalo." << std::endl;
        }
    }
    else {
        std::cerr << "[ERROR] Neznama funkce u -f: " << functionMode << std::endl;
        return 1;
    }

    if (result) {
        std::cout << "[INFO] Hotovo. Vystup: " << outputFile << std::endl;
        return 0;
    } else {
        return 1;
    }
}
