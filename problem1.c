#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

typedef unsigned int integer;
typedef unsigned long long natural;

typedef struct segment {
	integer a;
	integer b;
} segment;

typedef unsigned char bool;

void *pmalloc(size_t size) {
	void *block = malloc(size);
	if (block) {
		return block;
	}
	else {
		fprintf(stderr, "Could not allocate block of size %ld\n", size);
		exit(1);
		return NULL;
	}
}

void *prealloc(void *block, size_t size) {
	block = realloc(block, size);
	if (block) {
		return block;
	}
	else {
		fprintf(stderr, "Could not allocate block of size %ld\n", size);
		exit(1);
		return NULL;
	}
}

#define malloc pmalloc
#define realloc prealloc

#define INITIAL_CAP 16
#define LOAD_FACTOR 1.5f

//yeah im using globals, fight me
segment *segs;							//dynamic array
natural segcount;				//dynamic array count (how many are stored)
natural segcap;		//dynamic array capacity (how much room has been allocated)

void init() {
	if (segs) free(segs);
	segs = malloc(INITIAL_CAP * sizeof(segment));
	segcount = 0;
	segcap = INITIAL_CAP;
}

natural end() {
	natural total = 0;
	natural i;
	for (i = 0; i < segcount; i ++) {
		total += (segs[i].b - segs[i].a);
	}
	free(segs);
	segs = NULL;
	return total;
}

void adjust(natural newcount) {
	natural oldcap = segcap;
	while (newcount > segcap) {
		segcap *= LOAD_FACTOR;
	}
	if (oldcap != segcap) {
		realloc(segs, segcap);
	}
}

bool inside(integer x, segment s) {
	return (x >= s.a) && (x <= s.b);
}

void addseg(segment s) {
	//three cases:
	// 1: no overlap
	// 2: completely inside 1 segment, is consumed
	// 3: overlaps 1 segment, grows it
	// 4 overlaps 2 segments, joining them
	if (s.a == s.b) return; //zero length segments dont add anything
	natural i;
	for (i = 0; i < segcount; i ++) {
		segment *seg = segs + i;
		bool a_in = inside(s.a, *seg);
		bool b_in = inside(s.b, *seg);
		if (a_in && b_in) return;	// case 2: completely inside
		if (a_in) {
			seg->b = s.b;
			//case 3 or 4. check ahead for next segment
			// is there next segment to overlap?
			if ((i + 1) < segcount) {
				segment *next = seg + 1;
				if (inside(s.b, *next)) {
					// case 4: overlaps 2 segments. time to collapse
					seg->b = next->b;
					for (i ++; i < --segcount; i ++) {
						seg[i] = seg[i + 1];
					}
				}
				//otherwise, case #: 1 overlap, return
			}
			return;
		}
		if (b_in) {
			// case 3: 1 overlap. because it is the end that is inside, we know there is no chance of merging
			seg->a = s.a;
			return;
		}
	}
	//case 1: no overlap
	adjust(segcount + 1);
	segs[segcount ++] = s;
}

segment stoseg(char *s) {
		#define skipspace() for ( s = s; isspace(*s); s++) {}
		#define parseinteger(i) for (s = s; isdigit(*s); s ++) { *i *= 10; *i += *s-'0'; }
	segment seg;
	seg.a = 0;
	seg.b = 0;
	skipspace();
	parseinteger(&seg.a);
	skipspace();
	parseinteger(&seg.b);
	// make sure the endpoints are correctly ordered
	if (seg.b < seg.a) {
		integer c = seg.a;
		seg.a = seg.b;
		seg.b = c;
	}
	return seg;
		#undef skipspace
		#undef parseinteger
}

bool isblanks(char *s) {
	for (s = s; *s; s ++) {
		if (!isspace(*s)) {
			return 0;
		}
	}
	return 1;
}

int main(int argc, char **argv) {
	size_t n = 0;
	char *line;
	ssize_t status = getline(&line, &n, stdin);
	while (status > 0) {
		if (isblanks(line)) {
			//finish this run
			if (segs != NULL) {
				natural total = end();
				printf("%llu\n", total);
			}
		}
		else {
			if (!segs) {
				init();
			}
			segment seg = stoseg(line);
			printf("seg: a=%u b=%u\n", seg.a, seg.b);
			addseg(seg);
		}
		status = getline(&line, &n, stdin);
	}
	if (segs) end();
	return 0;
}
