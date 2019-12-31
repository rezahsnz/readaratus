SOURCES = src/main.c src/app.c src/rect.c src/toc.c src/find.c src/unit_convertor.c src/figure.c src/teleport_widget.c src/find_widget.c
CFLAGS = -Wall `pkg-config --cflags --libs gtk+-3.0 poppler-glib`
LDFLAGS = `pkg-config --libs gtk+-3.0 poppler-glib` -lm

readaratus : $(SOURCES)
	cc $(CFLAGS) -o build/readaratus $(SOURCES) $(LDFLAGS)

.PHONY : clean
clean :
	-rm build/readaratus
