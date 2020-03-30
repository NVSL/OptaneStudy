#!/usr/bin/python
import sys
from PyPDF2 import PdfFileReader, PdfFileWriter
from PyPDF2.pdf import PageObject
import StringIO
from reportlab.pdfgen import canvas

rows = 2
cols = (len(sys.argv) - 1) / rows
if rows * cols < (len(sys.argv) - 1):
    cols = cols + 1

if len(sys.argv) < 2:
    sys.exit()

reader = PdfFileReader(open(sys.argv[1],'rb'))
pages = reader.getNumPages()
pageWidth = reader.getPage(0).mediaBox.getWidth()
pageHeight = reader.getPage(0).mediaBox.getHeight()
print("Found a total of " + str(pages) + " pages.")

writer = PdfFileWriter()
for p in range(0, pages):
    page = PageObject.createBlankPage(None, pageWidth * cols, pageHeight * rows)
    for i in range(1, len(sys.argv)):
        myCol = (i - 1) % cols
        myRow = (i - 1) / cols

        reader = PdfFileReader(open(sys.argv[i],'rb'))
        temp = reader.getPage(p)
        page.mergeTranslatedPage(temp, myCol * pageWidth, myRow * pageHeight)

        packet = StringIO.StringIO()
        can = canvas.Canvas(packet)
        can.setPageSize(size=(pageWidth, pageHeight))
        can.drawString(65, 285, sys.argv[i])
        can.save()
        packet.seek(0)
        newPdf = PdfFileReader(packet)
        page.mergeTranslatedPage(newPdf.getPage(0), myCol * pageWidth, myRow * pageHeight)
    writer.addPage(page)
    print("Finished creating page " + str(p + 1))

with open('merged.pdf', 'wb') as f:
    writer.write(f)
