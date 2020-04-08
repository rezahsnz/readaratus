SOURCES = src/main.c src/app.c src/rect.c src/toc.c src/toc_synthesis.c src/find.c src/unit_convertor.c src/figure.c src/teleport_widget.c src/find_widget.c src/roman_numeral.c src/resource/resource.c
CFLAGS = -g -Wall `pkg-config --cflags --libs gtk+-3.0 poppler-glib`
LDFLAGS = `pkg-config --libs gtk+-3.0 poppler-glib` -lm

readaratus : $(SOURCES)
	cc $(CFLAGS) -o readaratus $(SOURCES) $(LDFLAGS)

.PHONY : clean
clean :
	-rm readaratus
