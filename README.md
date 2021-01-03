
<h1 align="center">readaratus - A dynamic book reading system for computers</h1>
<p align="center">

[![Watch the video](https://img.youtube.com/vi/j3546bj08Vk/maxresdefault.jpg)](https://www.youtube.com/watch?v=j3546bj08Vk)
</p> 

## Motives
Since the invention of the printing press we have seen little progress in the reading habits of humans. A physical book once published on paper or imitated on a computer, would not allow for its content to interact with the reader. No book, for example, would let the reader to turn any of its tables into an interactive chart despite the fact that it is quite difficult for humans to interpret large bodies of numbers. This immutable nature of paper curbs mental development of humans, but the situation is being changed by the arrival of computers. Humans(and other animals) learn by interacting with their environment and an environment rich in live elements would definitely boost the intellectual abilities of its reader. A computer is a medium that provides necessary tools needed to build a dynamic environment for any situation and there is an urgent need for lush, rich, and dynamic learning environments.<br/>readaratus is a tiny proof of concept and computerized experiment to introduce some dynamism into the realm of books. Ultimately, readaratus aims to be a "self-decoding", "dynamic", and "machine-friendly" protocol for learning.<br/>The following list demonstrates the current features of readaratus:
<ul>
	<p align="justify"><b>Unit conversion</b><br/>A minimal mechanism for converting units between SI and Imperial systems of measuring units.</p>
	<p align="justify"><b>Figure referencing</b><br/>It is common for pieces of text to be devoted to a some graphical representation. If a piece of text references a figure which does not reside in the same page, the reader has to manually find that particular figure in order to understand the point of the text. In physical books this is achieved by means of hands: groups of fingers dedicated to particular pages with intermittent activation of the page of interest. In a typical computer reading software this is done by frequent linking from the text to the figure and vice versa provided that the author has provided the link and the software provides facilitates for going back to where the user was. This process causes distortion in concentration whether the book is physical or digital. We have developed a mechanism for an on-demand invocation of figures wherever they are referenced. This mechanism is activated whenever the user moves his mouse pointer on top of the text that references a figure. Upon activation, the figure gets displayed on an overlaid box.</p>
	<p align="justify"><b>Teleportation</b><br/>Users have the freedom to jump to various objects within the book: Page numbers, page labels, TOC items(introduction, epilogue, chapter I, Section 2.4, ...), immediate neighbor TOC items(next chapter, previous subsection, ...), and figures. A dialog is prepared that lets the user to find any object.</p>
	<p align="justify"><b>Enhanced TOC</b><br/>We have developed an enhanced TOC subsystem that lets users to efficiently navigate within the book. TOC is read, decoded, grouped, and then absolute and relative means for navigation are provided. By absolute navigation we mean tools that facilitate jumping, for example, from "chapter 3" to "subsection 8.2", and by relative navigation we mean tools that aid in navigating to the neighboring "chapters", "sections", ... .</p>
	<p align="justify"><b>Dual-page text lookup</b><br/>A thin probabilistic proof of concept is provided in the "Find text" dialog where the user has the option of extending her textual search to two pages since it is common for a sentence to have some of its words in page "X" and the rest in page "X+1".</p>  
	<p align="justify"><b>Real page numbers</b><br/>Page numbers in PDF documents are either provided by the the authors or are indexed from 1. Indexed page numbers are often some pages ahead or behind the real page number(e.g. you are on page 45 but your software reports 72). We have developed means to overcome this issue and the page numbers you see or request are real as they are on paper.</p>
	<p align="justify"><b>TOC synthesis</b><br/>Some PDFs lack digitized "Table of Contents". If a book fails to provide a TOC, we try to create a synthetic TOC out of it. This is achieved by some analysis performed on initial pages of a PDF.</p>
</ul>

## Compile
Have the following dependencies installed:
 - `Poppler's GLib backend development package`
 - `Gtk3 SDK's development package`

Then open a shell and compile the application.
 ```bash
 make
 ```
 Once compilation finished, you can run the application.
 ```bash
 ./readaratus
 ```
## License
This project is licensed under the terms of GNU General Public License Version 3 or later - see the [LICENSE](LICENSE) file for details.
