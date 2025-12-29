<?php
/**
 * Cross-platform printer example
 * Works on both Windows and Linux
 */

// Check if the printer extension is loaded
if (!extension_loaded('printer')) {
    die("ERROR: Printer extension is not loaded.\n" .
        "Please install and enable the printer extension in php.ini\n");
}

echo "PHP Printer Extension - Cross-Platform Example\n";
echo "===============================================\n\n";

// List all available printers
echo "Available printers:\n";
try {
    $printers = printer_list(PRINTER_ENUM_LOCAL);
    
    if (empty($printers)) {
        echo "  No printers found.\n";
        echo "  Make sure you have at least one printer installed.\n\n";
        exit(1);
    }
    
    foreach ($printers as $index => $printer) {
        echo "  [$index] " . $printer['NAME'];
        if (isset($printer['IS_DEFAULT']) && $printer['IS_DEFAULT']) {
            echo " (default)";
        }
        echo "\n";
    }
    echo "\n";
} catch (Exception $e) {
    echo "ERROR listing printers: " . $e->getMessage() . "\n";
    exit(1);
}

// Open the default printer
echo "Opening default printer...\n";
$handle = printer_open();

if (!$handle) {
    die("ERROR: Failed to open printer connection.\n" .
        "Make sure a default printer is configured.\n");
}

echo "Printer opened successfully.\n\n";

// Prepare the content to print
$content = "Hello from PHP Printer Extension!\n";
$content .= "=================================\n\n";
$content .= "This is a cross-platform printing test.\n";
$content .= "Date: " . date('Y-m-d H:i:s') . "\n";
$content .= "Platform: " . PHP_OS . "\n";
$content .= "PHP Version: " . PHP_VERSION . "\n\n";
$content .= "This example works on both Windows and Linux systems.\n";

// Print the content
echo "Sending content to printer...\n";
$result = printer_write($handle, $content);

if ($result) {
    echo "Content sent successfully!\n";
    echo "Check your printer for output.\n";
} else {
    echo "ERROR: Failed to send content to printer.\n";
}

// Close the printer connection
echo "\nClosing printer connection...\n";
printer_close($handle);
echo "Done.\n";

?>
