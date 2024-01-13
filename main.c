#include <stdio.h>
#include <string.h>
#include <stdlib.h>
    
struct color {
    unsigned char b, g, r;
};

struct point {
    double x,y;
};

struct intRes {
    char *buf;
    int i;
};

struct intRes toInt(char *buf) {
    int i = 0, x = 0;
    for (; buf[x] >= '0' && buf[x] <= '9'; x++) {
        i = i * 10 + buf[x] - '0';
    }
    struct intRes res = {.buf = &(buf[x+1]), .i = i};
    return res;
}

void wFile(char *file, struct color* img, int w, int h) {
    FILE *f;
    int filesize = (54 + 3*w*h);

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

    fclose(f);
}

void evenOdd(char *area, int w, int h, struct point *line) {  
    for (int x = 0; x < w; x++) {
        for (int y = 0; y < h; y++) {
            if ((line[0].y > y) == (line[1].y > y)) continue;
            double slope =
                (x - line[0].x) * (line[1].y - line[0].y) - (line[1].x - line[0].x) * (y - line[0].y);
            if ((slope < 0) != (line[1].y < line[0].y)) area[x+y*w] = !area[x+y*w]; 
        }
    }
}

void rFile(char *file, struct color *img, int w, int h) {
    FILE *f = fopen(file,"r");
    char area[w*h], command[1024], stack[1024];
    int cmdI = 0, scopeSwitch = 0, afterCmd = 0;

    struct point line[2];

    memset(area,0,w*h);

    while (fgets(command,1024,f)) {
        char cmd = command[0];
        if (cmd == 'E') {
            afterCmd = 1;
            stack[cmdI] = '\0';
            cmdI--;
        } else if (cmd >= 'A' && cmd <= 'Z') {
            afterCmd = 1;
            stack[cmdI] = cmd;
            cmdI++;
        } else {
            struct intRes dat = toInt(command); 
            struct point thePoint = {
                .x = dat.i,
                .y = toInt(dat.buf).i
            };
           
            line[0] = line[1];
            line[1] = thePoint;

            if (afterCmd) {
                afterCmd = 0; 
                continue;
            }
    
            evenOdd(area, w, h, line);
     
        }
    }

    for (int i = 0; i < w*h; i++) {
        img[i].r = area[i] ? 255 : 0;
        img[i].g = area[i] ? 255 : 0;
        img[i].b = area[i] ? 255 : 0;
    }
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
    
    rFile(argv[1],img,w,h);

    wFile(argv[2],img,w,h);
}
