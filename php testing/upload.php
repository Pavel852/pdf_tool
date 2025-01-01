<?php
// upload.php
// Zpracovává nahrané soubory, spouští pdf_tool, vrací JSON odpověď.

header('Content-Type: application/json; charset=utf-8');

$targetDir = __DIR__ . '/uploads';
if (!is_dir($targetDir)) {
    mkdir($targetDir, 0777, true);
}

// Základní kontrola, zda přišly soubory a funkce
if (!isset($_POST['function']) || !isset($_FILES['files'])) {
    echo json_encode([
        'success' => false,
        'error' => 'Chybí parametry nebo soubory.'
    ]);
    exit;
}

$function = $_POST['function']; // "ocr" nebo "blend"
$uploadedFiles = $_FILES['files'];

// Uložíme nahrané soubory do /uploads/
$inputFiles = []; // pole cest k nahraným souborům
for ($i = 0; $i < count($uploadedFiles['name']); $i++) {
    $fileName = basename($uploadedFiles['name'][$i]);
    $tmpPath  = $uploadedFiles['tmp_name'][$i];
    if ($tmpPath && is_uploaded_file($tmpPath)) {
        $targetPath = $targetDir . '/' . uniqid() . '_' . $fileName;
        if (move_uploaded_file($tmpPath, $targetPath)) {
            $inputFiles[] = $targetPath;
        }
    }
}

// Pokud se nepodařilo žádný soubor uložit, končíme
if (count($inputFiles) === 0) {
    echo json_encode([
        'success' => false,
        'error' => 'Nepodařilo se uložit žádné soubory.'
    ]);
    exit;
}

// Připravíme výstupní soubor do /uploads/
if ($function === 'ocr') {
    // OCR -> výstupní .txt
    $outputFileName = 'ocr_result_' . time() . '.txt';
} else {
    // Blend -> výstupní .pdf
    $outputFileName = 'blend_result_' . time() . '.pdf';
}

$outputPath = $targetDir . '/' . $outputFileName;

// Sestavíme příkaz pro pdf_tool
// Pozn. Předpokládáme, že pdf_tool je v PATH, jinak použijte absolutní cestu.
$cmd = 'pdf_tool -f ' . escapeshellarg($function);

// Vložíme parametry -i (vstupní soubory)
$cmd .= ' -i';
foreach ($inputFiles as $file) {
    $cmd .= ' ' . escapeshellarg($file);
}

// Vložíme parametr -o (výstup)
$cmd .= ' -o ' . escapeshellarg($outputPath);

// Spustíme příkaz (pokud by to bylo dlouhé, je lepší použít proc_open atd.)
// 2>&1 přesměruje chyby do standardního výstupu, abychom je mohli vypsat
exec($cmd . ' 2>&1', $output, $returnVar);

if ($returnVar !== 0) {
    // Přišlo k chybě
    echo json_encode([
        'success' => false,
        'error' => 'pdf_tool selhal. Výstup: ' . implode("\n", $output)
    ]);
    exit;
}

// Pokud vše OK, vrátíme JSON s odkazem na stažení
// Sestavíme URL k souboru (předpokládáme, že kořen webu je o úroveň výš apod.)
$downloadUrl = 'uploads/' . $outputFileName;

echo json_encode([
    'success' => true,
    'downloadUrl' => $downloadUrl
]);
