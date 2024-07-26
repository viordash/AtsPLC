using System;
using System.Diagnostics;
using System.Text;

namespace Bmp2Ssd1306 {
    internal class Program {
        static void Main(string[] args) {
            if (args.Length < 1) {
                Console.WriteLine("Use: Bmp2Ssd1306 input.bmp [output.h] [-fv][-nc]");
                Console.WriteLine("\t-fv flip vertical");
                Console.WriteLine("\t-nc negative color");
                return;
            }

            var inputFilename = args[0];
            string outputFilename;
            if (args.Length >= 2 && Path.GetExtension(args[1]) == ".h") {
                outputFilename = args[1];
            } else {
                outputFilename = Path.ChangeExtension(inputFilename, ".h");
            }

            var bytes = File.ReadAllBytes(inputFilename);
            var roundedDib = ParseBmpFile(bytes);
            var dib = Convert2RawDib(roundedDib);

            if (args.Contains("-fv")) {
                FlipVertical(dib);
            }
            if (args.Contains("-nc")) {
                NegativeColor(dib);
            }
            var ssd1306 = Convert2Ssd1306(dib);
            var code = CreateCCodeArray(dib, ssd1306);
            File.WriteAllText(outputFilename, code);
            Console.WriteLine($"File '{outputFilename}' (w:{dib.Width}, h:{dib.Height}) ready");
        }

        static BitmapFile.Dib ParseBmpFile(byte[] bytes) {
            var dib = BitmapFile.ExtractDib(bytes);
            return dib;
        }

        static BitmapFile.Dib Convert2RawDib(BitmapFile.Dib dib) {
            var rawDib = new BitmapFile.Dib {
                Data = new byte[dib.Width / 8 * dib.Height],
                Height = dib.Height,
                Width = dib.Width
            };

            var width_aligned = ((dib.Width + 31) / 32) * 32;
            for (int row = 0; row < dib.Height; row++) {
                for (int column = 0; column < dib.Width / 8; column++) {
                    var id = row * (dib.Width / 8) + column;
                    var idRounded = row * (width_aligned / 8) + column;
                    rawDib.Data[id] = dib.Data[idRounded];
                }
            }
            return rawDib;
        }


        static void FlipVertical(BitmapFile.Dib dib) {
            for (int row = 0; row < dib.Height / 2; row++) {
                for (int column = 0; column < dib.Width / 8; column++) {
                    var id0 = row * (dib.Width / 8) + column;
                    var id1 = (dib.Height - 1 - row) * (dib.Width / 8) + column;
                    var t = dib.Data[id0];
                    dib.Data[id0] = dib.Data[id1];
                    dib.Data[id1] = t;
                }
            }
        }

        static void NegativeColor(BitmapFile.Dib dib) {
            for (int row = 0; row < dib.Height; row++) {
                for (int column = 0; column < dib.Width / 8; column++) {
                    var id = row * (dib.Width / 8) + column;
                    dib.Data[id] = (byte)~dib.Data[id];
                }
            }
        }

        static byte[] Convert2Ssd1306(BitmapFile.Dib dib) {
            var fb = new byte[(dib.Width / 8) * dib.Height];

            for (int row = 0; row < dib.Height; row++) {
                for (int column = 0; column < (dib.Width / 8); column++) {
                    var xbm_offset = (row * (dib.Width / 8)) + column;
                    for (byte bit = 0; bit < 8; bit++) {
                        if ((dib.Data[xbm_offset] & 1 << (7 - bit)) != 0) {
                            fb[dib.Width * (row / 8) + column * 8 + bit] |= (byte)(1 << row % 8);
                        }
                    }
                }
            }
            return fb;
        }

        static string CreateCCodeArray(BitmapFile.Dib dib, byte[] bytes) {
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
            sb.AppendLine("#include \"LogicProgram/InputNO.h\"");
            sb.AppendLine();
            sb.AppendLine("const Bitmap DisplayItemBase::bitmap = { //");
            sb.AppendLine($"    {{ {dib.Width},                                // width");
            sb.AppendLine($"      {dib.Height} }},                              // height");
            sb.AppendLine($"    {{ {string.Join(",\n", lines)} }}");
            sb.AppendLine("};");

            Debug.WriteLine(sb.ToString());
            return sb.ToString();
        }
    }
}