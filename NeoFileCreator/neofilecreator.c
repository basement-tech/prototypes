#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#define NUM_ARGS 6  //
#define MIN_ARGS 0  //
#define DELIM_START '<'
#define DELIM_STOP  '>'

#define MAX_HDR_SIZE 1024

static const char header_json[] =
R"==(
{"strategy" : "bitwise", "__comment" : "first line is skipped after noting strategy"}
{
    "label" : "USER-5",
    "strategy" : "bitwise",
    "bonus" : { "depth" : "2", "pixel_cnt" : 64, "brightness" : {"r": 64,  "g": 64, "b": 64, "w": 0}},
    "pixelcnt" : 32
}
)==";

char header[MAX_HDR_SIZE] = {0};;
char preamble[256] = {0};

typedef struct __attribute__((packed)) {
    int8_t o;  // offset
    uint32_t r, g, b, w;  // color bitmap
    int32_t d;   // delay
    uint16_t s;  // servo bitmap
    int32_t a;   // servo angle
} seq_bin_t;

//
// BEWARE OF STRUCTURE PACKING !!!
//                 Offset    R           G           B           W        T
seq_bin_t data[] = {
                        { 0, 0xA0A0A0A0, 0x00000000, 0x00000000, 0x00000000, 1000, 0x0001, 45},
                        { 1, 0xA0A0A0A0, 0x00000000, 0x00000000, 0x00000000, 1000, 0, 0},
                        { 0, 0xA0A0A0A0, 0x00000000, 0x00000000, 0x00000000, 1000, 0x0004, 45},
                        { 1, 0xA0A0A0A0, 0x00000000, 0x00000000, 0x00000000, 1000, 0, 0},
                        { 0, 0x50505050, 0x00000000, 0x00000000, 0x00000000, 1000, 0x0005, 0},
                        { 1, 0x50505050, 0x00000000, 0x00000000, 0x00000000, 1000, 0, 0},
                        { 0, 0x00000000, 0xA0A0A0A0, 0x00000000, 0x00000000, 1000, 0x0001, -45},
                        { 1, 0x00000000, 0xA0A0A0A0, 0x00000000, 0x00000000, 1000, 0, 0},
                        { 0, 0x00000000, 0xA0A0A0A0, 0x00000000, 0x00000000, 1000, 0x0004, -45},
                        { 1, 0x00000000, 0xA0A0A0A0, 0x00000000, 0x00000000, 1000, 0, 0},
                        { 0, 0x00000000, 0x50505050, 0x00000000, 0x00000000, 1000, 0x0005, 0},
                        { 1, 0x00000000, 0x50505050, 0x00000000, 0x00000000, 1000, 0, 0},
                        { 0, 0x00000000, 0x00000000, 0xA0A0A0A0, 0x00000000, 1000, 0x0001, 10},
                        { 1, 0x00000000, 0x00000000, 0xA0A0A0A0, 0x00000000, 1000, 0, 0},
                        { 0, 0x00000000, 0x00000000, 0x50505050, 0x00000000, 1000, 0x0001, 0},
                        { 1, 0x00000000, 0x00000000, 0x50505050, 0x00000000, 1000, 0, 0},
                        { 0, 0x01010101, 0x00000000, 0x00000000, 0x00000000, -1, 0, 0},
                        { 1, 0x01010101, 0x00000000, 0x00000000, 0x00000000, -1, 0, 0}
                    };

void main(int argc, char **argv)  {
    char filename[128] = "neo_user_6.bseq";
    char label[32] = "USER-6";
    char strategy[32] = "bbitwise";
    char bonus[] = "{ \"depth\" : 2, \"pixel_cnt\" : 64, \"brightness\" : {\"r\": 64,  \"g\": 64, \"b\": 64, \"w\": 0}}";
    char pixelcnt[32] = "32";
    int jlen = 0; // length of the json to follow

    FILE *fp;
    int num = 0;
    int err = 0;
    char arg = '\0';

    if(argc < (MIN_ARGS+1))
        fprintf(stderr, "Error: too few args; filename required\n");
    else if(argc > (NUM_ARGS+1))
        fprintf(stderr, "Error: Too many args ... more than %d\n", NUM_ARGS);
    else  {
        bool needhyphen = true;
        argv++;--argc;
        while((argc-- > 0) && (err >= 0))  {
            if(needhyphen == true)  {
                if((*argv)[0] != '-')  {
                    fprintf(stderr, "Error: malformed arguments\n");
                    err = -1;
                }
                else  {
                    switch((*argv)[1])  {
                        case 'f':  // filename
                        case 'p':  // pixelcount
                        case 's':  // strategy
                            arg = (*argv)[1];
                            break;

                        default:
                            fprintf(stderr, "Error: unknown argument");
                            err = -1;
                            break;
                    }
                    argv++;
                    needhyphen = false;
                }
            }
            else  {
                switch(arg)  {
                    case 'f':  // filename
                        strncpy(filename, *argv, sizeof(filename));
                        break;

                    case 'p':  // pixelcount
                        strncpy(pixelcnt, *argv, sizeof(pixelcnt));
                        break;

                    case 's':  // strategy
                        strncpy(strategy, *argv, sizeof(strategy));
                        break;

                    default:
                        fprintf(stderr, "Error: unknown argument");
                        err = -1;
                        break;
                }
                argv++;
                needhyphen = true;
            }
        }
        printf("Using:\n");
        printf("   filename :  %s\n", filename);
        printf("   pixelcount: %s\n", pixelcnt);
        printf("   strategy:   %s\n", strategy);

        if(err == 99999)  {
            if((fp = fopen(filename, "wb")) == NULL)
                fprintf(stderr, "Error: can't open file %s\n", filename);
            else
            {
                uint32_t cbal = sizeof(header);

                snprintf(header, cbal, "{\n");
                snprintf(header+strlen(header), (cbal-=strlen(header)), "  \"label\" : \"%s\",\n", label);
                snprintf(header+strlen(header), (cbal-=strlen(header)), "  \"strategy\" : \"%s\",\n", strategy);
                snprintf(header+strlen(header), (cbal-=strlen(header)), "  \"bonus\" : %s,\n", bonus);
                snprintf(header+strlen(header), (cbal-=strlen(header)), "  \"__comment\" : \"back and forth\"\n");
                snprintf(header+strlen(header), (cbal-=strlen(header)), "}\n");

                uint32_t hdr_len = strlen(header);
                printf("Header as buffer:\n%s\n", header);
                printf("strlen(header) = %d\n", strlen(header));

                snprintf(preamble, cbal, "{\"filetype\" : \"BIN_BW\", \"jsonlen\" : %4u, \"__comment\" : \"points as binary\"}\n", hdr_len);
                fwrite(preamble, sizeof(char), strlen(preamble), fp);

                num = fwrite(header, sizeof(char), strlen(header), fp);
                printf("%d header characters written\n", num);
                printf("sizeof() each binary data structure = %d\n", sizeof(seq_bin_t));
                num = fwrite(data, sizeof(uint8_t), sizeof(data), fp);
                printf("%d binary bytes written\n", num);
                fclose(fp);
            }
        }
    }
}