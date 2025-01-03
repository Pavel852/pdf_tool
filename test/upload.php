<?php
// Nastavte cestu k pdf_tool (zkompilovaný binární soubor):
// Např. na Linuxu: "/usr/local/bin/pdf_tool"
// Např. Windows (WSL/Cygwin): "/mnt/c/Users/xxxx/pdf_tool.exe"
$pdfToolPath = "../pdf_tool";

// Nastavení hlavičky pro JSON výstup
header('Content-Type: application/json; charset=utf-8');

// Adresář pro nahrané a výstupní soubory
$targetDir = __DIR__ . '/uploads';
if (!is_dir($targetDir)) {
    mkdir($targetDir, 0777, true);
}

// Ověříme, zda přišla funkce (ocr|blend) a nějaké soubory
if (!isset($_POST['function']) || !isset($_FILES['files'])) {
    echo json_encode([
        'success' => false,
        'error'   => 'Chybí parametry nebo soubory.'
    ]);
    exit;
}

$function = $_POST['function']; // "ocr" nebo "blend"
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

// Pokud se nepodařilo žádný soubor uložit, skončíme
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

// Sestavíme příkaz pro pdf_tool
$cmd  = escapeshellcmd($pdfToolPath);
// Důležitá oprava: argument musí být '-f=ocr' nebo '-f=blend'
$cmd .= ' -f=' . escapeshellarg($function);

// Přidáme -i a jednotlivé vstupní soubory
$cmd .= ' -i';
foreach ($inputFiles as $file) {
    $cmd .= ' ' . escapeshellarg($file);
}

// Nastavíme -o s cestou k výstupnímu souboru
$cmd .= ' -o ' . escapeshellarg($outputPath);

// Spustíme příkaz a zjistíme výstup a návratový kód
exec($cmd . ' 2>&1', $output, $returnVar);

// Pokud není návratový kód 0, došlo k chybě
if ($returnVar !== 0) {
    echo json_encode([
        'success' => false,
        'error'   => 'pdf_tool selhalo. Výstup: ' . implode("\n", $output)
    ]);
    exit;
}

// Sestavíme URL k výstupnímu souboru (relativní odkaz z pohledu prohlížeče)
$downloadUrl = 'uploads/' . $outputFileName;

echo json_encode([
    'success'     => true,
    'downloadUrl' => $downloadUrl
]);
