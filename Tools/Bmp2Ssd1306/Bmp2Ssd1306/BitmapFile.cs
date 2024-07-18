using System.Runtime.InteropServices;

namespace Bmp2Ssd1306 {
    public class BitmapFile {
        public struct Dib {
            public byte[] Data;
            public int Width;
            public int Height;
        }

        [StructLayout(LayoutKind.Sequential, Pack = 2)]
        public struct BITMAPFILEHEADER {
            public ushort bfType;
            public uint bfSize;
            public ushort bfReserved1;
            public ushort bfReserved2;
            public uint bfOffBits;

            public BITMAPFILEHEADER() {
                bfSize = StructHelper.Size(this);
            }
        }

        public static Dib ExtractDib(byte[] bytesBmp) {
            if (bytesBmp.Length < StructHelper.Size<BITMAPFILEHEADER>()) {
                throw new ArgumentException("bmp size is too small");
            }

            var header = StructHelper.FromBytes<BITMAPFILEHEADER>(bytesBmp);
            if (bytesBmp.Length < header.bfSize) {
                throw new ArgumentException("wrong bmp size");
            }
            if (header.bfType != 0x4D42) {
                throw new ArgumentException("incorrect bmp type");
            }

            var bmp = bytesBmp.Skip((int)StructHelper.Size<BITMAPFILEHEADER>()).ToArray();
            if (!BITMAPINFO.TryParse(bmp, out BITMAPINFO bitmapInfo)) {
                if (!BITMAPV5INFO.TryParse(bmp, out BITMAPV5INFO bitmapV5Info)) {
                    throw new ArgumentException("bmp info parse error");
                }
                if (bitmapV5Info.bmiHeader.bV5BitCount != 1) {
                    throw new ArgumentException("incorrect bmp bit count");
                }
            } else {
                if (bitmapInfo.bmiHeader.biBitCount != 1) {
                    throw new ArgumentException("incorrect bmp bit count");
                }
            }
            return new Dib {
                Data = bytesBmp.Skip((int)header.bfOffBits).ToArray(),
                Height = bitmapInfo.bmiHeader.biHeight,
                Width = bitmapInfo.bmiHeader.biWidth

            };
        }
    }
}
