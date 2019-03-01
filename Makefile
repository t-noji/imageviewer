iv: get_pixbuf_webp_from_file.o get_pixbuf_heif_from_file.o widget_func.o iv.o
	gcc -Wall -O2 `pkg-config --libs gtk+-3.0 gmodule-2.0` -o iv -lwebp -lheif -no-pie widget_func.o get_pixbuf_webp_from_file.o get_pixbuf_heif_from_file.o iv.o
get_pixbuf_webp_from_file.o: get_pixbuf_webp_from_file.c
	gcc -Wall -O2 `pkg-config --cflags gtk+-3.0` -c get_pixbuf_webp_from_file.c
get_pixbuf_heif_from_file.o: get_pixbuf_heif_from_file.c
	gcc -Wall -O2 `pkg-config --cflags gtk+-3.0` -c get_pixbuf_heif_from_file.c
widget_func.o: widget_func.c iv.h
	gcc -Wall -O2 `pkg-config --cflags gtk+-3.0 gmodule-2.0` -c widget_func.c
iv.o: iv.c iv.h
	gcc -Wall -O2 `pkg-config --cflags gtk+-3.0` -c iv.c
