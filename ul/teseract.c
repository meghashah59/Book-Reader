#include <stdio.h>
#include <stdlib.h>
#include <tesseract/capi.h>
#include <leptonica/allheaders.h>


int main() {
    TessBaseAPI* handle = TessBaseAPICreate();
    TessBaseAPIInit3(handle, NULL, "eng");

    // Load image
    struct Pix* image = pixRead("image.jpg");
    TessBaseAPISetImage2(handle, image);

    // Perform OCR
    TessBaseAPIRecognize(handle, NULL);

    // Get the result
    char* result = TessBaseAPIGetUTF8Text(handle);
    printf("OCR Result:\n%s\n", result);

    // Clean up
    TessDeleteText(result);
    pixDestroy(&image);
    TessBaseAPIDelete(handle);

    return 0;
}

