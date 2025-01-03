/*
 * pdf_tool.cpp
 *
 * Version: 1.0
 * Author:  PB, pavel.bartos.pb@gmail.com
 * License: CC
 *
 * Nástroj demonstrující sloučení PDF/obrázků do jednoho PDF (pomocí PDFium)
 * a OCR z PDF/obrázků do textu (pomocí Tesseract).
 */

#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <cstdio>

// PDFium
#include "public/fpdfview.h"
#include "public/fpdf_edit.h"
#include "public/fpdf_save.h"
#include "public/fpdf_ppo.h"

// Tesseract
#include <tesseract/baseapi.h>
#include <leptonica/allheaders.h>

/**
 * Inicializace PDFium. Je vhodné ji zavolat na začátku programu.
 */
void initializePDFium() {
    FPDF_InitLibrary();
}

/**
 * Deinicializace PDFium. Zavolat před ukončením programu.
 */
void destroyPDFium() {
    FPDF_DestroyLibrary();
}

/**
 * Vytvoří PDF z daného obrázku (PNG/JPG) s využitím PDFium:
 *   - Vytvoří nový PDF dokument
 *   - Vytvoří jednu stránku
 *   - Vloží obrázek jako bitmapu do stránky
 *   - Uloží takto vytvořený PDF do dočasného souboru (return string).
 *
 * Pozn: Ve skutečnosti je práce s PDFium pro obrázky poměrně nízkoúrovňová
 * (FPDFPageObj_NewImageObj, FPDFImageObj_SetBitmap, atd.), proto je tahle funkce
 * pouze schematická.
 */
std::string createPDFfromImage(const std::string& imagePath) {
    // 1) Vytvoříme nový dokument
    FPDF_DOCUMENT doc = FPDF_CreateNewDocument();
    if (!doc) {
        std::cerr << "[ERROR] Nelze vytvorit novy PDF dokument pro " << imagePath << std::endl;
        return "";
    }

    // 2) Přidáme stránku, např. velikost A4 (612 x 792 body = 8.5" x 11")
    FPDF_PAGE page = FPDFPage_New(doc, 0, 612, 792);
    if (!page) {
        std::cerr << "[ERROR] Nelze vytvorit stranku v PDF dokumentu (image: " << imagePath << ")" << std::endl;
        FPDF_CloseDocument(doc);
        return "";
    }

    // 3) Vytvoříme image objekt
    FPDF_PAGEOBJECT imgObj = FPDFPageObj_NewImageObj(doc);
    if (!imgObj) {
        std::cerr << "[ERROR] Nelze vytvorit image objekt (image: " << imagePath << ")" << std::endl;
        FPDF_ClosePage(page);
        FPDF_CloseDocument(doc);
        return "";
    }

    // 4) Načteme obrázek do bitmapy
    FPDF_BITMAP bitmap = FPDFBitmap_CreateFromFile(imagePath.c_str());
    if (!bitmap) {
        std::cerr << "[ERROR] Nelze nacist bitmapu z souboru " << imagePath << std::endl;
        FPDFPageObj_Destroy(imgObj);
        FPDF_ClosePage(page);
        FPDF_CloseDocument(doc);
        return "";
    }

    // Nastavíme bitmapu do image objektu
    FPDFImageObj_SetBitmap(&page, 0, imgObj, bitmap);

    // Nastavíme velikost obrázku na stránce (jednoduše 500 x 500 bodů)
    // V praxi byste řešili poměr stran a velikost.
    FPDFPageObj_Transform(imgObj, 500.0, 0, 0, 500.0, 50.0, 100.0);

    // 5) Vložíme objekt do stránky
    FPDFPage_InsertObject(page, imgObj);

    // 6) Uložíme stránku
    FPDFPage_GenerateContent(page);

    // 7) Uložíme dokument do dočasného PDF souboru
    //    Můžeme použít např. unikátní jméno souboru
    static int counter = 0;  // pro generování unikátních jmen
    std::string tempPDF = "temp_image_" + std::to_string(counter++) + ".pdf";

    if (!FPDF_SaveAsCopy(doc, tempPDF.c_str(), FPDF_NO_INCREMENTAL)) {
        std::cerr << "[ERROR] Nelze ulozit docasny PDF pro " << imagePath << std::endl;
        // úklid
        FPDFBitmap_Destroy(bitmap);
        FPDF_ClosePage(page);
        FPDF_CloseDocument(doc);
        return "";
    }

    // úklid
    FPDFBitmap_Destroy(bitmap);
    FPDF_ClosePage(page);
    FPDF_CloseDocument(doc);

    return tempPDF;
}

/**
 * Sloučí zadané PDF/PNG/JPG soubory do jednoho PDF (pomocí PDFium).
 * - PNG/JPG se nejprve konvertují (každý) do dočasného PDF.
 * - Poté se PDF soubory sloučí do jednoho výstupního PDF.
 */
bool mergeFilesIntoPDF(const std::vector<std::string>& inputFiles, const std::string& outputFile) {
    // 1) Vytvoříme cílový PDF dokument
    FPDF_DOCUMENT dstDoc = FPDF_CreateNewDocument();
    if (!dstDoc) {
        std::cerr << "[ERROR] Nelze vytvorit vystupni PDF dokument: " << outputFile << std::endl;
        return false;
    }

    // 2) Pro každý vstupní soubor:
    //    - Pokud končí na .pdf, přímo importujeme.
    //    - Pokud je to obrázek, nejprve konvertujeme createPDFfromImage().

    for (auto &file : inputFiles) {
        // Malá funkce pro ověření přípony
        auto toLower = [](std::string s) {
            std::transform(s.begin(), s.end(), s.begin(), ::tolower);
            return s;
        };
        std::string lowerFile = toLower(file);

        std::string pdfToImport = file;
        if (lowerFile.rfind(".pdf") == std::string::npos) {
            // není .pdf => předpokládáme .png/.jpg nebo jiný obrázek
            pdfToImport = createPDFfromImage(file);
            if (pdfToImport.empty()) {
                std::cerr << "[ERROR] Chyba pri konverzi " << file << " na PDF." << std::endl;
                FPDF_CloseDocument(dstDoc);
                return false;
            }
        }

        // Nyní máme PDF (původní nebo dočasné), importujeme do dstDoc
        FPDF_DOCUMENT srcDoc = FPDF_LoadDocument(pdfToImport.c_str(), nullptr);
        if (!srcDoc) {
            std::cerr << "[ERROR] Nelze nacist PDF dokument: " << pdfToImport << std::endl;
            FPDF_CloseDocument(dstDoc);
            return false;
        }

        // Import všech stránek srcDoc do dstDoc:
        // PDFium API: FPDF_ImportPages(targetDoc, sourceDoc, "stranky", insertIndex)
        // "stranky" lze definovat jako "1-3", "2", atd. My chceme všechny stránky: 
        //   Můžeme tedy zjistit počet stránek a udělat si takový string "1-<page_count>"
        int pageCount = FPDF_GetPageCount(srcDoc);
        std::string pageRange = "1-" + std::to_string(pageCount);

        if (!FPDF_ImportPages(dstDoc, srcDoc, pageRange.c_str(), FPDF_GetPageCount(dstDoc))) {
            std::cerr << "[ERROR] Nelze importovat stranky z " << pdfToImport << std::endl;
            FPDF_CloseDocument(srcDoc);
            FPDF_CloseDocument(dstDoc);
            return false;
        }

        FPDF_CloseDocument(srcDoc);
    }

    // 3) Uložíme výsledný dokument
    if (!FPDF_SaveAsCopy(dstDoc, outputFile.c_str(), FPDF_NO_INCREMENTAL)) {
        std::cerr << "[ERROR] Chyba pri ukladani vystupniho PDF: " << outputFile << std::endl;
        FPDF_CloseDocument(dstDoc);
        return false;
    }

    FPDF_CloseDocument(dstDoc);
    return true;
}

/**
 * Provede OCR přes Tesseract pro dané PDF/PNG/JPG soubory a uloží do jednoho textového souboru.
 * - Pokud je PDF, můžeme buď využít extrakci stránek do bitmap (komplikované),
 *   nebo (zjednodušeně) jen "věřit", že PDF obsahuje bitmapy (reálné PDF OCR je složitější).
 *   Zde použijeme "dump" všech stran do PNG a projedeme je Tesseractem.
 *   (PDFium umožňuje renderovat stránky na FPDF_BITMAP, tu pak konvertovat do PNG.)
 * - Pokud je obrázek, přímo ho nakrmíme Tesseractem.
 *
 * Zde to **zjednodušíme**: 
 *   - Pro PNG/JPG přímo zpracujeme Tesseractem.
 *   - Pro PDF: "pseudo-kód" k vykreslení stránek do bitmapy a poté zpracování Tesseractem.
 * Ve výsledku text zapíšeme (či připojíme) do `outputFile`.
 */
bool performOCR(const std::vector<std::string>& inputFiles, const std::string& outputFile) {
    // Otevřeme výstupní soubor pro zápis
    FILE* out = fopen(outputFile.c_str(), "w");
    if (!out) {
        std::cerr << "[ERROR] Nelze otevrit vystupni soubor pro zapis: " << outputFile << std::endl;
        return false;
    }

    // Inicializace Tesseractu
    tesseract::TessBaseAPI ocr;
    // Předpokládáme, že jazykové modely (např. "eng") jsou v defaultní cestě
    if (ocr.Init(nullptr, "eng")) {
        std::cerr << "[ERROR] Nelze inicializovat Tesseract (eng)" << std::endl;
        fclose(out);
        return false;
    }

    for (auto &file : inputFiles) {
        // Rozlišíme PDF vs obrázek:
        auto toLower = [](std::string s) {
            std::transform(s.begin(), s.end(), s.begin(), ::tolower);
            return s;
        };
        std::string lowerFile = toLower(file);

        if (lowerFile.rfind(".pdf") != std::string::npos) {
            // PDF => pro každou stránku:
            FPDF_DOCUMENT doc = FPDF_LoadDocument(file.c_str(), nullptr);
            if (!doc) {
                std::cerr << "[ERROR] Nelze nacist PDF dokument (OCR): " << file << std::endl;
                continue;
            }
            int pageCount = FPDF_GetPageCount(doc);

            for (int pageIndex = 0; pageIndex < pageCount; pageIndex++) {
                FPDF_PAGE page = FPDF_LoadPage(doc, pageIndex);
                if (!page) {
                    std::cerr << "[WARN] Nelze nacist stranku " << pageIndex << " (PDF: " << file << ")" << std::endl;
                    continue;
                }

                // Zjistíme rozměry stránky (width x height)
                double width = FPDF_GetPageWidth(page);
                double height = FPDF_GetPageHeight(page);

                // Vytvoříme bitmapu pro Tesseract
                //  -> Počítejme DPI ~ 150 (zjednodušeno).
                int dpi = 150;
                int bitmapW = static_cast<int>(width * dpi / 72.0);  // PDFium default je 72 DPI
                int bitmapH = static_cast<int>(height * dpi / 72.0);

                FPDF_BITMAP bitmap = FPDFBitmap_Create(bitmapW, bitmapH, 0);
                if (!bitmap) {
                    std::cerr << "[WARN] Nelze vytvorit bitmapu (page " << pageIndex << " z " << file << ")" << std::endl;
                    FPDF_ClosePage(page);
                    continue;
                }

                // Vykreslení stránky do bitmapy
                //  - bílé pozadí (0xFFFFFFFF)
                FPDF_DWORD fillColor = 0xFFFFFFFF;
                FPDFBitmap_FillRect(bitmap, 0, 0, bitmapW, bitmapH, fillColor);
                FPDF_RenderPageBitmap(bitmap, page, 0, 0, bitmapW, bitmapH, 0, 0);

                // Nyní máme data v FPDF_BITMAP => Tesseract nepotřebuje PNG soubor,
                // stačí mu přímo pointer na pixely. Nicméně pro Tesseract je nejjednodušší
                // vytvořit Pix (Leptonica). Narychlo lze data kopírovat do Pix.
                // ZDE: Pro zjednodušení vytvoříme Pix z bitmapy:
                unsigned char* buffer = (unsigned char*)FPDFBitmap_GetBuffer(bitmap);
                // PDFium defaultně dává BGRA (4 bajty na pixel).
                // Tesseract obvykle očekává 8-bit (grayscale) nebo 24-bit (RGB).
                // Uděláme tedy konverzi BGRA -> 8bit grayscale (abychom nemuseli složitě
                // do Tesseractu posílat 32bit RGBA).
                Pix* pix = pixCreate(bitmapW, bitmapH, 8); // grayscale
                if (pix) {
                    l_uint32* pixData = pixGetData(pix);
                    int pixWpl = pixGetWpl(pix);

                    // Pro každý pixel:
                    for (int y = 0; y < bitmapH; y++) {
                        for (int x = 0; x < bitmapW; x++) {
                            int idx = (y * bitmapW + x) * 4; // BGRA
                            unsigned char b = buffer[idx + 0];
                            unsigned char g = buffer[idx + 1];
                            unsigned char r = buffer[idx + 2];
                            // jednoduchý grayscale
                            unsigned char gray = (unsigned char)(0.3*r + 0.59*g + 0.11*b);
                            // Uložit do pixData
                            SET_DATA_BYTE(pixData, x, gray);
                        }
                        pixData += pixWpl;
                    }
                    // Po naplnění předáme Tesseractu
                    ocr.SetImage(pix);
                    char* ocrResult = ocr.GetUTF8Text();
                    if (ocrResult) {
                        // Zapíšeme do výstupního .txt souboru
                        fprintf(out, "===== [File: %s | Page: %d] =====\n", file.c_str(), pageIndex + 1);
                        fprintf(out, "%s\n", ocrResult);
                        delete[] ocrResult;
                    }
                    pixDestroy(&pix);
                }

                FPDFBitmap_Destroy(bitmap);
                FPDF_ClosePage(page);
            }

            FPDF_CloseDocument(doc);
        }
        else {
            // Obrázek => přímo Tesseract
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
    std::cout << "pdf_tool - A simple CLI for PDF and image manipulation (PDFium + Tesseract).\n"
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
    // Inicializace PDFium (na začátku programu)
    initializePDFium();

    if (argc < 2) {
        // Bez parametru - vypíšeme nápovědu
        printHelp();
        destroyPDFium();
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
        destroyPDFium();
        return 0;
    }

    // Ošetření -v / --version
    if (doVersion) {
        printVersion();
        destroyPDFium();
        return 0;
    }

    // Kontrola, zda máme nějakou funkci k vykonání
    if (functionMode.empty()) {
        std::cerr << "[ERROR] Nebyl zadan parametr -f=blend|ocr" << std::endl;
        destroyPDFium();
        return 1;
    }

    // Kontrola, zda máme nějaké input soubory
    if (inputFiles.empty()) {
        std::cerr << "[ERROR] Nebyly zadany vstupni soubory pomoci -i" << std::endl;
        destroyPDFium();
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
        destroyPDFium();
        return 1;
    }

    // Deinicializace PDFium
    destroyPDFium();

    if (result) {
        std::cout << "[INFO] Hotovo. Vystup: " << outputFile << std::endl;
        return 0;
    } else {
        return 1;
    }
}
