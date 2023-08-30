#include <stdio.h>

/*
Copyright (c) 2021-2023 Devine Lu Linvega, Andrew Alderwick

Permission to use, copy, modify, and distribute this software for any
purpose with or without fee is hereby granted, provided that the above
copyright notice and this permission notice appear in all copies.

THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
WITH REGARD TO THIS SOFTWARE.
*/

#define TRIM 0x0100   //0x0100  256
#define LENGTH 0x10000  // 0x10000   65536

typedef unsigned char Uint8;
typedef signed char Sint8;
typedef unsigned short Uint16;
/**
* In programming, Macro (macro) is a way for automation that allows you to define a piece of code and use a name to refer to this piece of code.
* Then in the program, every time this name is used, the compiler will automatically replace it with the defined code segment.
*In the code you gave, Macro is defined as a structure containing the following fields:
*char name[0x40]: The name of the macro, stored in a character array with a length of 64 (0x40 is equal to 64 in hexadecimal).
*char items[0x40][0x40]: Macro items, which is a two-dimensional character array,
*Can store 64 strings of length 64, possibly representing the maximum number of entries a macro can contain.
*Uint8 len: The length of the macro, possibly indicating the number of entries the macro contains.
 */
typedef struct {
	char name[0x40], items[0x40][0x40];
	Uint8 len;
} Macro;
/**
* In assembly language, a label is a symbol or name used to identify a specific location or piece of code.
* When writing assembly code, you may need to set labels at certain locations in the code so that these locations can be referenced elsewhere.
* start:          ; This is a label
*    MOV AX, 10  ; Assembly instruction
*    JMP start   ; Jump to the 'start' label
*
 */
typedef struct {
	char name[0x40];
	Uint16 addr, refs;
} Label;
/**
* char name[0x40]: This is a character array with a length of 64 (0x40 is equal to 64 in hexadecimal), used to store the referenced name.
* char rune: A character whose exact meaning and use depends on the context of your program, but from some hints from other code snippets, this may be a character used to identify a specific type of reference.
* Uint16 addr: This is a 16-bit unsigned integer used to store the address of the reference.
 */
typedef struct {
	char name[0x40], rune;
	Uint16 addr;
} Reference;
/**
* Uint8 data[LENGTH]: This field is an array for storing machine code or assembly code.

*unsigned int ptr, length: These two fields may be used to track the usage of the data array.

*ptr may be the next location to write data to, and length may indicate the number of used bytes in data.

*Uint16 llen, mlen, rlen: These three fields may represent the number of labels (Label), macros (Macro) and references (Reference) in the program respectively.

*Label labels[0x400]: This field is an array, storing all labels in the program.

*Macro macros[0x100]: This field is an array that stores all the macros in the program.

*Reference refs[0x800]: This field is an array that stores all references in the program.

*char scope[0x40]: This field may indicate the current scope or context, such as the name of the function or module currently being parsed.

 */
typedef struct {
	Uint8 data[LENGTH];
	unsigned int ptr, length;
	Uint16 llen, mlen, rlen;
	Label labels[0x400];
	Macro macros[0x100];
	Reference refs[0x800];
	char scope[0x40];
} Program;

Program p;

/* clang-format off */

static char ops[][4] = {
	"LIT", "INC", "POP", "NIP", "SWP", "ROT", "DUP", "OVR",
	"EQU", "NEQ", "GTH", "LTH", "JMP", "JCN", "JSR", "STH",
	"LDZ", "STZ", "LDR", "STR", "LDA", "STA", "DEI", "DEO",
	"ADD", "SUB", "MUL", "DIV", "AND", "ORA", "EOR", "SFT"
};

static int   scmp(char *a, char *b, int len) { int i = 0; while(a[i] == b[i]) if(!a[i] || ++i >= len) return 1; return 0; } /* string compare */
static int   sihx(char *s) { int i = 0; char c; while((c = s[i++])) if(!(c >= '0' && c <= '9') && !(c >= 'a' && c <= 'f')) return 0; return i > 1; } /* string is hexadecimal */
static int   shex(char *s) { int n = 0, i = 0; char c; while((c = s[i++])) if(c >= '0' && c <= '9') n = n * 16 + (c - '0'); else if(c >= 'a' && c <= 'f') n = n * 16 + 10 + (c - 'a'); return n; } /* string to num */
static int   slen(char *s) { int i = 0; while(s[i]) i++; return i; } /* string length */
static int   spos(char *s, char c) { Uint8 i = 0, j; while((j = s[i++])) if(j == c) return i; return -1; } /* character position */
static char *scpy(char *src, char *dst, int len) { int i = 0; while((dst[i] = src[i]) && i < len - 2) i++; dst[i + 1] = '\0'; return dst; } /* string copy */
static char *scat(char *dst, const char *src) { char *ptr = dst + slen(dst); while(*src) *ptr++ = *src++; *ptr = '\0'; return dst; } /* string cat */

/* clang-format on */

static int parse(char *w, FILE *f);

static int
error(const char *name, const char *msg)
{
	fprintf(stderr, "%s: %s\n", name, msg);
	return 0;
}

static char *
sublabel(char *src, char *scope, char *name)
{
	if(slen(scope) + slen(name) >= 0x3f) {
		error("Sublabel length too long", name);
		return NULL;
	}
	return scat(scat(scpy(scope, src, 0x40), "/"), name);
}

static Macro *
findmacro(char *name)
{
	int i;
	for(i = 0; i < p.mlen; i++)
		if(scmp(p.macros[i].name, name, 0x40))
			return &p.macros[i];
	return NULL;
}

static Label *
findlabel(char *name)
{
	int i;
	for(i = 0; i < p.llen; i++)
		if(scmp(p.labels[i].name, name, 0x40))
			return &p.labels[i];
	return NULL;
}

static Uint8
findopcode(char *s)
{
	int i;
	for(i = 0; i < 0x20; i++) {
		int m = 0;
		if(!scmp(ops[i], s, 3))
			continue;
		if(!i) i |= (1 << 7); /* force keep for LIT */
		while(s[3 + m]) {
			if(s[3 + m] == '2')
				i |= (1 << 5); /* mode: short */
			else if(s[3 + m] == 'r')
				i |= (1 << 6); /* mode: return */
			else if(s[3 + m] == 'k')
				i |= (1 << 7); /* mode: keep */
			else
				return 0; /* failed to match */
			m++;
		}
		return i;
	}
	return 0;
}

static int
makemacro(char *name, FILE *f)
{
	Macro *m;
	char word[0x40];
	if(findmacro(name))
		return error("Macro duplicate", name);
	if(sihx(name) && slen(name) % 2 == 0)
		return error("Macro name is hex number", name);
	if(findopcode(name) || scmp(name, "BRK", 4) || !slen(name))
		return error("Macro name is invalid", name);
	if(p.mlen == 0x100)
		return error("Macros limit exceeded", name);
	m = &p.macros[p.mlen++];
	scpy(name, m->name, 0x40);
	while(fscanf(f, "%63s", word) == 1) {
		if(word[0] == '{') continue;
		if(word[0] == '}') break;
		if(word[0] == '%')
			return error("Macro error", name);
		if(m->len >= 0x40)
			return error("Macro size exceeded", name);
		scpy(word, m->items[m->len++], 0x40);
	}
	return 1;
}

static int
makelabel(char *name)
{
	Label *l;
	if(findlabel(name))
		return error("Label duplicate", name);
	if(sihx(name) && (slen(name) == 2 || slen(name) == 4))
		return error("Label name is hex number", name);
	if(findopcode(name) || scmp(name, "BRK", 4) || !slen(name))
		return error("Label name is invalid", name);
	if(p.llen == 0x400)
		return error("Labels limit exceeded", name);
	l = &p.labels[p.llen++];
	l->addr = p.ptr;
	l->refs = 0;
	scpy(name, l->name, 0x40);
	return 1;
}

static int
makereference(char *scope, char *label, char rune, Uint16 addr)
{
	char subw[0x40], parent[0x40];
	Reference *r;
	if(p.rlen >= 0x800)
		return error("References limit exceeded", label);
	r = &p.refs[p.rlen++];
	if(label[0] == '&') {
		if(!sublabel(subw, scope, label + 1))
			return error("Invalid sublabel", label);
		scpy(subw, r->name, 0x40);
	} else {
		int pos = spos(label, '/');
		if(pos > 0) {
			Label *l;
			if((l = findlabel(scpy(label, parent, pos))))
				l->refs++;
		}
		scpy(label, r->name, 0x40);
	}
	r->rune = rune;
	r->addr = addr;
	return 1;
}

static int
writebyte(Uint8 b)
{
	if(p.ptr < TRIM)
		return error("Writing in zero-page", "");
	else if(p.ptr > 0xffff)
		return error("Writing after the end of RAM", "");
	else if(p.ptr < p.length)
		return error("Memory overwrite", "");
	p.data[p.ptr++] = b;
	p.length = p.ptr;
	return 1;
}

static int
writeopcode(char *w)
{
	return writebyte(findopcode(w));
}

static int
writeshort(Uint16 s, int lit)
{
	if(lit)
		if(!writebyte(findopcode("LIT2"))) return 0;
	return writebyte(s >> 8) && writebyte(s & 0xff);
}

static int
writelitbyte(Uint8 b)
{
	return writebyte(findopcode("LIT")) && writebyte(b);
}
/**
* Function doinclude, it receives a string parameter filename, this string represents a file name. The main function of this function is to open the file, parse each string in the file one by one (that is, each word or tag), and then close the file.

*The following are the specific steps of this function:

*Use the fopen function to open the file in read-only mode ("r").

*If the file does not exist, the fopen function returns NULL. At this time, the error function is called to report an error and return.

*If the file is opened successfully, it uses a while loop to read strings from the file through the fscanf function, one at a time, up to 63 characters.

*For each read string w, call the parse function for processing. The parse function has been defined before and is used to parse strings and perform corresponding operations.

*If the parse function returns failure (returns 0), the doinclude function also returns failure and calls the error function to report an error.。
 * @param filename
 * @return
 */
static int
doinclude(const char *filename)
{
	FILE *f;
	char w[0x40];
	if(!(f = fopen(filename, "r")))
		return error("Include missing", filename);
	while(fscanf(f, "%63s", w) == 1)
		if(!parse(w, f))
			return error("Unknown token", w);
	fclose(f);
	return 1;
}
/**
 * This function receives two parameters, one is char *w and the other is FILE *f. char *w is a pointer to a character, possibly the word or token to be parsed.
 *FILE *f is a pointer to a file that may contain code or data to be parsed.
 * @param w
 * @param f
 * @return
 */
static int
parse(char *w, FILE *f)
{
	int i;
	char word[0x40], subw[0x40], c;
	Label *l;
	Macro *m;
	if(slen(w) >= 63)
		return error("Invalid token", w);
	switch(w[0]) {
	case '(': /* comment */
		if(slen(w) != 1) fprintf(stderr, "-- Malformed comment: %s\n", w);
		i = 1; /* track nested comment depth */
		while(fscanf(f, "%63s", word) == 1) {
			if(slen(word) != 1)
				continue;
			else if(word[0] == '(')
				i++;
			else if(word[0] == ')' && --i < 1)
				break;
		}
		break;
	case '~': /* include */
		if(!doinclude(w + 1))
			return error("Invalid include", w);
		break;
	case '%': /* macro */
		if(!makemacro(w + 1, f))
			return error("Invalid macro", w);
		break;
	case '|': /* pad-absolute */
		if(sihx(w + 1))
			p.ptr = shex(w + 1);
		else if(w[1] == '&') {
			if(!sublabel(subw, p.scope, w + 2) || !(l = findlabel(subw)))
				return error("Invalid sublabel", w);
			p.ptr = l->addr;
		} else {
			if(!(l = findlabel(w + 1)))
				return error("Invalid label", w);
			p.ptr = l->addr;
		}
		break;
	case '$': /* pad-relative */
		if(sihx(w + 1))
			p.ptr += shex(w + 1);
		else if(w[1] == '&') {
			if(!sublabel(subw, p.scope, w + 2) || !(l = findlabel(subw)))
				return error("Invalid sublabel", w);
			p.ptr += l->addr;
		} else {
			if(!(l = findlabel(w + 1)))
				return error("Invalid label", w);
			p.ptr += l->addr;
		}
		break;
	case '@': /* label */
		if(!makelabel(w + 1))
			return error("Invalid label", w);
		scpy(w + 1, p.scope, 0x40);
		break;
	case '&': /* sublabel */
		if(!sublabel(subw, p.scope, w + 1) || !makelabel(subw))
			return error("Invalid sublabel", w);
		break;
	case '#': /* literals hex */
		if(sihx(w + 1) && slen(w) == 3)
			return writelitbyte(shex(w + 1));
		else if(sihx(w + 1) && slen(w) == 5)
			return writeshort(shex(w + 1), 1);
		else
			return error("Invalid hex literal", w);
		break;
	case '_': /* raw byte relative */
		makereference(p.scope, w + 1, w[0], p.ptr);
		return writebyte(0xff);
	case ',': /* literal byte relative */
		makereference(p.scope, w + 1, w[0], p.ptr + 1);
		return writelitbyte(0xff);
	case '-': /* raw byte absolute */
		makereference(p.scope, w + 1, w[0], p.ptr);
		return writebyte(0xff);
	case '.': /* literal byte zero-page */
		makereference(p.scope, w + 1, w[0], p.ptr + 1);
		return writelitbyte(0xff);
	case ':':
	case '=': /* raw short absolute */
		makereference(p.scope, w + 1, w[0], p.ptr);
		return writeshort(0xffff, 0);
	case ';': /* literal short absolute */
		makereference(p.scope, w + 1, w[0], p.ptr + 1);
		return writeshort(0xffff, 1);
	case '?': /* JCI */
		makereference(p.scope, w + 1, w[0], p.ptr + 1);
		return writebyte(0x20) && writeshort(0xffff, 0);
	case '!': /* JMI */
		makereference(p.scope, w + 1, w[0], p.ptr + 1);
		return writebyte(0x40) && writeshort(0xffff, 0);
	case '"': /* raw string */
		i = 0;
		while((c = w[++i]))
			if(!writebyte(c)) return 0;
		break;
	case '[':
	case ']':
		if(slen(w) == 1) break; /* else fallthrough */
	default:
		/* opcode */
		if(findopcode(w) || scmp(w, "BRK", 4))
			return writeopcode(w);
		/* raw byte */
		else if(sihx(w) && slen(w) == 2)
			return writebyte(shex(w));
		/* raw short */
		else if(sihx(w) && slen(w) == 4)
			return writeshort(shex(w), 0);
		/* macro */
		else if((m = findmacro(w))) {
			for(i = 0; i < m->len; i++)
				if(!parse(m->items[i], f))
					return 0;
			return 1;
		} else {
			makereference(p.scope, w, ' ', p.ptr + 1);
			return writebyte(0x60) && writeshort(0xffff, 0);
		}
	}
	return 1;
}

static int
resolve(void)
{   // label
	Label *l;
	int i;
	Uint16 a;
	for(i = 0; i < p.rlen; i++) {
		Reference *r = &p.refs[i];
		switch(r->rune) {
            //For the case of '_', ',': it thinks it is a relative reference, so it calculates the relative offset (the difference between the tag address and the reference address minus 2).
            // If the difference is too large to fit into a Sint8, an error will be returned. This reference is then stored in the Sint8 variable p.data at address r->addr
		case '_':
		case ',':
			if(!(l = findlabel(r->name)))
				return error("Unknown relative reference", r->name);
			p.data[r->addr] = (Sint8)(l->addr - r->addr - 2);
			if((Sint8)p.data[r->addr] != (l->addr - r->addr - 2))
				return error("Relative reference is too far", r->name);
			l->refs++;
			break;
                // For the case of "-", ".": it is a zero page reference, so the lower 8 bits of the label address are stored in r->addr p.data.
		case '-':
		case '.':
			if(!(l = findlabel(r->name)))
				return error("Unknown zero-page reference", r->name);
			p.data[r->addr] = l->addr & 0xff;
			l->refs++;
			break;
                // For the case of ":", "=", ";": it is an absolute reference.
                // Here, the address of the tag is stored in p.data in the form of a 16-bit big endian number (high byte first, then low byte). r->addr
		case ':':
		case '=':
		case ';':
			if(!(l = findlabel(r->name)))
				return error("Unknown absolute reference", r->name);
			p.data[r->addr] = l->addr >> 0x8;
			p.data[r->addr + 1] = l->addr & 0xff;
			l->refs++;
			break;
                //For the case of "?", "!": It is also an absolute reference, but the difference between the label address and the reference address minus 2 is stored as a 16-bit big-endian number.
		case '?':
		case '!':
		default:
			if(!(l = findlabel(r->name)))
				return error("Unknown absolute reference", r->name);
			a = l->addr - r->addr - 2;
			p.data[r->addr] = a >> 0x8;
			p.data[r->addr + 1] = a & 0xff;
			l->refs++;
			break;
		}
	}
	return 1;
}
/**
 *
* char w[0x40];w declares a character array of size 0x40 (decimal 64). This will save every tag read from the assembly file.
  * p.ptr = 0x100; Set the ptr field of a global structure to 0x100 (decimal 256). p and its ptr fields are not defined in this snippet, but based on common patterns in assembler, ptr can represent a memory pointer or similar concept.
  * scpy("on-reset", p.scope, 0x40); seems to be copying the string "on-reset" to the scope field of p, presumably using a safe copy function that doesn't overflow the destination.
  * while(fscanf(f, "%62s", w) == 1) reads up to 62 non-whitespace characters f from the file at a time and stores them in w. It continues until fscanf cannot read the token so far, this may be because it has reached the end of the file.
  * if(slen(w) > 0x3d || !parse(w, f)) checks each read token. slen(w) roughly calculates the string length w of , 0x3d is 61 in decimal. If the length of the token exceeds 61, or if parse(w, f) returns 0 (possibly indicating that parse failed), call error("Invalid token", w) and return its result.
  * If it traverses the entire file without errors, return resolve(); is executed. The resolve function is not defined in this snippet, but in assembler such a function is usually responsible for resolving symbols such as the address of a label.
 * @param f
 * @return
 */
static int
assemble(FILE *f)
{
	char w[0x40];
	p.ptr = 0x100;
	scpy("on-reset", p.scope, 0x40);
	while(fscanf(f, "%62s", w) == 1)
		if(slen(w) > 0x3d || !parse(w, f))
			return error("Invalid token", w);
	return resolve();
}
/**
* The function is to traverse all labels and print out some assembly code statistical information.
* Loop through the p.labels array, which stores all labels.
* If the first character of the label is uppercase (that is, the ASCII code is between 'A' and 'Z'), then ignore it and enter the next loop.

* If a label is not referenced (p.labels[i].refs has a value of 0), then print the label's name to the standard error output stream, prefixed with -- Unused label:.

* Finally, a summary message is printed to the standard error output stream containing the following information:

* The filename of the assembly.
* The assembled byte size (p.length - TRIM) and the percentage of the 6502 CPU's memory capacity that this size occupies. The 6502 is an 8-bit microprocessor that can address up to 64KB of memory, so divide the total size by 652.80 (one percent of 64KB) to calculate the percentage used.
* Total number of tags (p.llen).
* Total macro count (p.mlen).
 * @param filename
 */
static void
review(char *filename)
{
	int i;
	for(i = 0; i < p.llen; i++)
		if(p.labels[i].name[0] >= 'A' && p.labels[i].name[0] <= 'Z')
			continue; /* Ignore capitalized labels(devices) */
		else if(!p.labels[i].refs)
			fprintf(stderr, "-- Unused label: %s\n", p.labels[i].name);
	fprintf(stderr,
		"Assembled %s in %d bytes(%.2f%% used), %d labels, %d macros.\n",
		filename,
		p.length - TRIM,
		(p.length - TRIM) / 652.80,
		p.llen,
		p.mlen);
}

static void
writesym(char *filename)
{
	int i;
	char symdst[0x60];
	FILE *fp;
	if(slen(filename) > 0x60 - 5)
		return;
	fp = fopen(scat(scpy(filename, symdst, slen(filename) + 1), ".sym"), "w");
	if(fp != NULL) {
		for(i = 0; i < p.llen; i++) {
			Uint8 hb = p.labels[i].addr >> 8, lb = p.labels[i].addr & 0xff;
			fwrite(&hb, 1, 1, fp);
			fwrite(&lb, 1, 1, fp);
			fwrite(p.labels[i].name, slen(p.labels[i].name) + 1, 1, fp);
		}
	}
	fclose(fp);
}

int
main(int argc, char *argv[])
// argc holds a count of arguments provided on the command line, and argv is an array of these arguments
{ // src (short for source, meaning source) and dst (short for destination, meaning target)
	FILE *src, *dst;
    // argc represents the count of command line arguments, and argv is an array of these arguments. The first element of the array (index 0) is always the name of the program itself, so if argc is less than 3, it means not enough arguments were provided
    // Because the normal program call should be like this: "uxnasm input.tal output.rom", where "uxnasm" is the program name, "input.tal" and "output.rom" are two parameters, so there should be a total of 3 command line parameters.
	if(argc < 3)
		return !error("usage", "uxnasm input.tal output.rom");
	if(!(src = fopen(argv[1], "r")))
		return !error("Invalid input", argv[1]);
	if(!assemble(src))
		return !error("Assembly", "Failed to assemble rom.");
	if(!(dst = fopen(argv[2], "wb")))
		return !error("Invalid Output", argv[2]);
	if(p.length <= TRIM)
		return !error("Assembly", "Output rom is empty.");
	fwrite(p.data + TRIM, p.length - TRIM, 1, dst);
	review(argv[2]);
	writesym(argv[2]);
	return 0;
}
