#include <stdio.h>
#include <string.h>
#include <stdlib.h>

struct color {
    unsigned char b, g, r;
};

struct intRes {
    char *buf;
    int i;
};

struct intRes toInt(char *buf) {
    int i = 0;
    for (int x = 0; buf[x] >= 48 && buf[x] <= 57; x++) {
        i = (i) * 10 + buf[x] - 48;
    }
    struct intRes res = {.buf = buf, .i = i};
    return res;
}

int wFile(char *file, struct color* img, int w, int h) {
    FILE *f;
    int filesize = (54 + 3*w*h);
    img = malloc(3*w*h);
    memset(img,0,3*w*h);

    unsigned char bmpfileheader[14];
    memset(bmpfileheader,0,14);
    strcpy(bmpfileheader, "BM");
    bmpfileheader[10] = 54;

    unsigned char bmpinfoheader[40];
    memset(bmpinfoheader,0,40); 
    bmpinfoheader[0] = 40;
    bmpinfoheader[12] = 1;
    bmpinfoheader[14] = 24;
    
    unsigned char bmppad[3];

    memcpy(&bmpfileheader[2], &filesize, 4);
    memcpy(&bmpinfoheader[4], &w, 4); 
    memcpy(&bmpinfoheader[8], &h, 4);

    f = fopen(file,"wb");
    fwrite(bmpfileheader,1,14,f);
    fwrite(bmpinfoheader,1,40,f);
    for(int i=0; i<h; i++)
    {
        fwrite(img+(w*(h-i-1)),3,w,f);
        fwrite(bmppad,1,(4-(w*3)%4)%4,f);
    }

    free(img);
    fclose(f);
}

int main(int argc, char **argv) {
    if (argc < 5) {
        printf("usage: %s [input file] [output file] [width] [height]\n",argv[0]);
        return -1;
    }
    int w, h;

    w = toInt(argv[3]).i;
    h = toInt(argv[4]).i;

    struct color img[w*h];   

    wFile(argv[2],img,w,h);
}
