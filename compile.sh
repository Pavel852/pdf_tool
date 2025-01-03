sudo apt-get update
sudo apt-get install -y build-essential imagemagick libqpdf-dev libtesseract-dev libleptonica-dev tesseract-ocr tesseract-ocr-eng

g++ pdf_tool.cpp -o pdf_tool -std=c++11 \
    -I/opt/pdfium/include -L/opt/pdfium/lib -lpdfium \
    -I/usr/include/tesseract -ltesseract -llept

