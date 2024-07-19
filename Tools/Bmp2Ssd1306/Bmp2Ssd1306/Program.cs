using System;
using System.Diagnostics;
using System.Text;

namespace Bmp2Ssd1306 {
    internal class Program {
        static void Main(string[] args) {
            if (args.Length != 2) {
                Console.WriteLine("Use: Bmp2Ssd1306 input.bmp output.h");
            }

            var inputFilename = args[0];
            var outputFilename = args[1];
            var outputArrayName = Path.GetFileNameWithoutExtension(outputFilename);

            var bytes = File.ReadAllBytes(inputFilename);
            var dib = ParseBmpFile(bytes);
            var ssd1306 = Convert2Ssd1306(dib);
            var code = CreateCCodeArray(ssd1306, outputArrayName);
            File.WriteAllText(outputFilename, code);
            Console.WriteLine($"File '{outputFilename}' ready");
        }

        static BitmapFile.Dib ParseBmpFile(byte[] bytes) {
            var dib = BitmapFile.ExtractDib(bytes);
            return dib;
        }

        static byte[] Convert2Ssd1306(BitmapFile.Dib dib) {
            var fb = new byte[dib.Width * dib.Height / 8];

            for (int row = 0; row < dib.Height; row++) {
                for (int column = 0; column < dib.Width / 8; column++) {
                    var xbm_offset = row * 16 + column;
                    for (byte bit = 0; bit < 8; bit++) {
                        if ((dib.Data[xbm_offset] & 1 << (7 - bit)) != 0) {
                            fb[dib.Width * (row / 8) + column * 8 + bit] |= (byte)(1 << row % 8);
                        }
                    }
                }
            }
            return fb;
        }

        static string CreateCCodeArray(byte[] bytes, string outputArrayName) {
            var lines = new List<string>();
            for (int row = 0; row < (bytes.Length + 15) / 16; row++) {
                var colsCount = Math.Min(16, bytes.Length - row * 16);
                lines.Add(string.Join(", ", bytes
                    .Skip(row * 16)
                    .Take(colsCount)
                    .Select(x => "0x" + x.ToString("X2"))
                    ));
            }

            var sb = new StringBuilder();
            sb.AppendLine($"uint8_t {outputArrayName}[] = {{");
            sb.AppendLine(string.Join(",\n", lines));
            sb.AppendLine("};");

            Debug.WriteLine(sb.ToString());
            return sb.ToString();

        }
    }
}