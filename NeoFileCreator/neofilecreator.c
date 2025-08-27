#include <stdio.h>
#include <string.h>
#include <stdint.h>

#define NUM_ARGS 1  // filename
#define MIN_ARGS 1  // filename required
#define DELIM_START '<'
#define DELIM_STOP  '>'

static const char header_json[] =
R"==(
{
    "label" : "USER-5",
    "strategy" : "bitwise",
    "bonus" : { "depth" : "2", "pixel_cnt" : 32, "brightness" : {"r": 255,  "g": 0, "b": 0, "w": 0}}
}
)==";

char *header = "{\"label\":\"USER-5\", }";
//                       R           G           B           W        T
uint32_t data[] = { 0x10101010, 0x00000000, 0x00000000, 0x00000000, 1000,
                    0x01010101, 0x00000000, 0x00000000, 0x00000000, 1000,
                    0x0, 0x0, 0x0, 0x0, -1 };

void main(int argc, char **argv)  {
    char filename[128];
    FILE *fp;
    int num = 0;

    if(argc < (MIN_ARGS+1))
        fprintf(stderr, "Error: too few args; filename required\n");
    else if(argc > (NUM_ARGS+1))
        fprintf(stderr, "Error: Too many args ... more than %d\n", NUM_ARGS);
    else  {
        for(int n = 1; n < argc; n++)  {
            printf("Using filename : %s\n", argv[n]);
            switch(n)  {
                case 0:
                    break;
                case 1:
                    strncpy(filename, argv[n], sizeof(filename));
                    break;
                default:
                    break;
            }
        }
        if((fp = fopen(filename, "wb")) == NULL)
            fprintf(stderr, "Error: can't open file %s\n", filename);
        else
        {
            fprintf(fp, "%c%d%c", DELIM_START, strlen(header_json), DELIM_STOP);
            printf("wrote size of header as %d\n", strlen(header_json));
            num = fwrite(header_json, sizeof(uint8_t), strlen(header_json), fp);
            printf("%d characters written\n", num);
            num = fwrite(data, sizeof(uint8_t), sizeof(data), fp);
            printf("%d bytes written\n", num);
            fclose(fp);
        }
    }
}