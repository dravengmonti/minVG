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

void wFile(char *file, struct color* img, size_t w, size_t h) {
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

void evenOdd(char *area, size_t w, size_t h, struct point *line) {  
    for (size_t x = 0; x < w; x++) {
        for (size_t y = 0; y < h; y++) {
            if ((line[0].y > y) == (line[1].y > y)) continue;
        
            double slope =
                (x - line[0].x) * (line[1].y - line[0].y) - (line[1].x - line[0].x) * (y - line[0].y);
            if ((slope < 0) != (line[1].y < line[0].y)) area[x+y*w] = !area[x+y*w]; 
        }
    }
}

void approxCurve(char *area, size_t w, size_t h, struct point *line) {
    struct point p1 = line[0];
    for (int i = 0; i < 64; i++) {
        double t = i / 64.0;
        double t2 = 1.0 - t;
    
        struct point p2 = p1; 
        p1.x = t2 * t2 * t2 * line[0].x + t2*t2*3*t * line[1].x + t2*3*t*t * line[2].x + t*t*t*line[3].x;
        p1.y = t2 * t2 * t2 * line[0].y + t2*t2*3*t * line[1].y + t2*3*t*t * line[2].y + t*t*t*line[3].y; 

        struct point line2[2];
        line2[0] = p1;
        line2[1] = p2;

        evenOdd(area, w, h, line2); 
    }
}

void rFile(char *file, struct color *img, size_t w, size_t h) {
    FILE *f = fopen(file,"r");
    char *area, command[1024], lastCmd = 'E';
    int cmdI = 0, scopeSwitch = 0, afterCmd = 0;

    struct point line[4];

    area = malloc(w*h);

    memset(area,0,w*h);
    memset(img,0,3*w*h);

    while (fgets(command,1024,f)) {
        char cmd = command[0];
        if (cmd == 'P') {
            afterCmd = 3;
        } else if (cmd == 'L') {
            afterCmd = 1;
        } else if (cmd == 'N') {
        } else if (cmd >= 'A' && cmd <= 'Z'){
            afterCmd = 0;
        } else if (cmd == ' ') {

        } else if (lastCmd == 'N') {
            struct intRes dat = toInt(command); 
            struct intRes dat2 = toInt(dat.buf);
            
            struct color rgb = {
                .r = dat.i,
                .g = dat2.i,
                .b = toInt(dat2.buf).i
            };

            for (int i = 0; i < w*h; i++) {
                img[i].r = area[i] ? rgb.r : img[i].r;
                img[i].g = area[i] ? rgb.g : img[i].g;
                img[i].b = area[i] ? rgb.b : img[i].b;
                area[i] = 0;
            } 
        } else {
            struct intRes dat = toInt(command); 
            struct point thePoint = {
                .x = dat.i,
                .y = toInt(dat.buf).i
            };

            if (lastCmd == 'L') {
                line[0] = line[1];
                line[1] = thePoint;
                if (afterCmd > 0) {
                    afterCmd--; 
                    continue;
                }
                evenOdd(area,w,h,line);
            } else if (lastCmd == 'P') { 
                line[0] = line[1];
                line[1] = line[2];
                line[2] = line[3];
                line[3] = thePoint;
                if (afterCmd > 0) {
                    afterCmd--; 
                    continue;
                }
                afterCmd = 3;
                approxCurve(area, w, h, line);
            }
            continue;
        }
        lastCmd = cmd;
    }
    for (size_t i = 0; i < w*h; i++) {
        img[i].r = area[i] ? 255 : img[i].r;
        img[i].g = area[i] ? 255 : img[i].g;
        img[i].b = area[i] ? 255 : img[i].b;
    }
    free(area);
}

int main(int argc, char **argv) {
    if (argc < 5) {
        printf("usage: %s [input file] [output file] [width] [height]\n",argv[0]);
        return -1;
    }
    size_t w, h;

    w = toInt(argv[3]).i;
    h = toInt(argv[4]).i;

    struct color *img = malloc(sizeof(struct color) * w * h);   
    
    rFile(argv[1],img,w,h);

    wFile(argv[2],img,w,h);
   
    free(img);
}
