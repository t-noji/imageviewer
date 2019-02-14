# 特徴  
GTK3  
ライトウエイト画像ビューア  
WebP対応  
HEIF対応  
高解像度モニタ対応  
# Build  
libwebpとlibgtk-3をインストールした上で  
``gcc -c readWebPFile.c``  
``gcc `pkg-config --cflags --libs gtk+-3.0 gmodule-2.0` -c get_pixbuf_heif_from_file.c``  
``gcc `pkg-config --cflags --libs gtk+-3.0 gmodule-2.0` -c widget_func.c``  
``gcc `pkg-config --cflags --libs gtk+-3.0 gmodule-2.0` -o iv -lwebp -lheif -no-pie readWebPFile.o widget_func.o iv.c``  
