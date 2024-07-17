using System;

namespace Bmp2Ssd1306 {
    internal class Program {
        static void Main(string[] args) {
            if (args.Count() != 2) {
                Console.WriteLine("Use: Bmp2Ssd1306 input.bmp output.h");
            }

            var inputFilename = args[0];
            var outputFilename = args[1];

            var bytes = File.ReadAllBytes(inputFilename);
            parseBmpFile(bytes);


            Console.WriteLine("Hello World!");
        }

        static void parseBmpFile(byte[] bytes) {
            var dib = BitmapFile.ExtractDib(bytes);
            // if (!BitmapFile.ExtractDib(bytes, out BITMAPINFO bitmapInfo)) {
            //     throw new ArgumentException("bmp file parse error");
            // }
            // if (bitmapInfo.bmiHeader.biBitCount != 1) {
            //     throw new ArgumentException("wrong bmp file format");
            // }
        }
    }
}