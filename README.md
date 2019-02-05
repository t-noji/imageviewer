# 特徴  
GTK3  
ライトウエイト画像ビューア  
WebP対応  
高解像度モニタ対応  
# Build  
libwebpとlibgtk-3をインストールした上で  
$ gcc -c readWebPFile.c  
$ gcc -o iv ``pkg-config --cflags --libs gtk+-3.0 gmodule-2.0`` -lwebp -no-pie readWebPFile.o iv.c  

