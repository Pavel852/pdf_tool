<?php
// Nastavte dle skutečné cesty k pdf_tool (binární soubor):
$pdfToolPath = "../pdf_tool";
// Příklad pro Linux: "/usr/local/bin/pdf_tool"
// Příklad pro Windows (cygwin/WSL): "/mnt/c/Users/xxx/pdf_tool.exe"

header('Content-Type: application/json; charset=utf-8');

$targetDir = __DIR__ . '/uploads';
if (!is_dir($targetDir)) {
    mkdir($targetDir, 0777, true);
}

// Kontrola, zda přišly soubory a funkce
if (!isset($_POST['function']) || !isset($_FILES['files'])) {
    echo json_encode([
        'success' => false,
        'error'   => 'Chybí parametry nebo soubory.'
    ]);
    exit;
}

$function     = $_POST['function']; // "ocr" nebo "blend"
$uploadedFiles = $_FILES['files'];

// Uložíme nahrané soubory do /uploads/
$inputFiles = [];
for ($i = 0; $i < count($uploadedFiles['name']); $i++) {
    $fileName = basename($uploadedFiles['name'][$i]);
    $tmpPath  = $uploadedFiles['tmp_name'][$i];

    if ($tmpPath && is_uploaded_file($tmpPath)) {
        // Vytvoříme jedinečný název (uniqid)
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
        'error'   => 'Nepodařilo se uložit žádné soubory.'
    ]);
    exit;
}

// Připravíme název výstupního souboru
if ($function === 'ocr') {
    $outputFileName = 'ocr_result_' . time() . '.txt';
} else {
    // Předpokládáme blend -> .pdf
    $outputFileName = 'blend_result_' . time() . '.pdf';
}

$outputPath = $targetDir . '/' . $outputFileName;

// Sestavíme příkaz. Použijeme escapeshellcmd (pro binárku) a escapeshellarg (pro argumenty).
$cmd  = escapeshellcmd($pdfToolPath);
$cmd .= ' -f ' . escapeshellarg($function);
$cmd .= ' -i';
foreach ($inputFiles as $file) {
    $cmd .= ' ' . escapeshellarg($file);
}
$cmd .= ' -o ' . escapeshellarg($outputPath);

// Spustíme příkaz a získáme výstup a návratový kód
exec($cmd . ' 2>&1', $output, $returnVar);

if ($returnVar !== 0) {
    // Chyba
    echo json_encode([
        'success' => false,
        'error'   => 'pdf_tool selhalo. Výstup: ' . implode("\n", $output)
    ]);
    exit;
}

// Sestavíme URL k souboru, abychom ho mohli stáhnout
// Předpokládáme, že "uploads" je dostupné na stejné úrovni jako "upload.php"
$downloadUrl = 'uploads/' . $outputFileName;

// Vrátíme JSON
echo json_encode([
    'success'     => true,
    'downloadUrl' => $downloadUrl
]);

