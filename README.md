# 特徴  
GTK3版ライトウエイト画像ビューア  
WebP対応  
高解像度モニタ対応  
# Build  
gcc -c readWebPFile.c  
gcc -o iv `pkg-config --cflags --libs gtk+-3.0 gmodule-2.0` -lwebp -no-pie readWebPFile.o iv.c  

